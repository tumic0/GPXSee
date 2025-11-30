#include <QPainter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPixmapCache>
#include <QImageReader>
#include "MVT/style_mvt.h"
#include "osm.h"
#include "pmtilesmap.h"

#define MVT_TILE_SIZE   512
#define MAX_TILE_SIZE   4096
#define LEAF_CACHE_SIZE 16

using namespace PMTiles;
using namespace MVT;
using namespace OSM;

PMTilesMap::PMTilesMap(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _file(fileName), _style(0), _mapRatio(1.0),
  _tileRatio(1.0), _hillShading(false), _mvt(false), _valid(false)
{
	if (!_file.open(QIODevice::ReadOnly)) {
		_errorString = _file.errorString();
		return;
	}

	// header
	Header hdr;
	if (!readHeader(_file, hdr, _errorString))
		return;

	_bounds = RectC(pos(hdr.minLon, hdr.maxLat), pos(hdr.maxLon, hdr.minLat));
	if (!_bounds.isValid()) {
		_errorString = "Invalid map bounds";
		return;
	}

	for (int i = hdr.minZ; i <= hdr.maxZ; i++)
		_zoomsBase.append(Zoom(i, i));
	if (_zoomsBase.isEmpty()) {
		_errorString = "Invalid zoom levels range";
		return;
	}
	_zoom = _zoomsBase.size() - 1;

	_tileOffset = hdr.tileOffset;
	_leafOffset = hdr.leafOffset;
	_tc = hdr.tc;
	_ic = hdr.ic;

	// metadata
	if (hdr.metadataLength) {
		QByteArray uba(readData(_file, hdr.metadataOffset, hdr.metadataLength,
		  hdr.ic));
		if (uba.isNull())
			qWarning("%s: error reading metadata", qUtf8Printable(fileName));
		else {
			QJsonParseError error;
			QJsonDocument doc(QJsonDocument::fromJson(uba, &error));
			if (doc.isNull())
				qWarning("%s: metadata error: %s", qUtf8Printable(fileName),
				  qUtf8Printable(error.errorString()));
			else {
				QJsonObject json(doc.object());
				_name = json["name"].toString();
				QJsonArray vl(json["vector_layers"].toArray());
				for (int i = 0; i < vl.size(); i++)
					_layers.append(vl.at(i).toObject()["id"].toString());
			}
		}
	}

	// root directory
	_root = readDir(_file, hdr.rootOffset, hdr.rootLength, hdr.ic);
	if (_root.isEmpty()) {
		_errorString = "Error reading root directory";
		return;
	}

	// tile info
	if (hdr.tt == 1) {
		_tileSize = MVT_TILE_SIZE;
		_mvt = true;
	} else {
		QByteArray data((_tc == 2)
		  ? Util::gunzip(tileData(_root.first().tileId))
		  : tileData(_root.first().tileId));
		QBuffer buffer(&data);
		QImageReader reader(&buffer);

		QSize tileSize(reader.size());
		if (!tileSize.isValid() || tileSize.width() != tileSize.height()) {
			_errorString = "Unsupported/invalid tile images";
			return;
		}
		_tileSize = tileSize.width();
	}

	_file.close();

	_cache.setMaxCost(LEAF_CACHE_SIZE);

	_valid = true;
}

void PMTilesMap::load(const Projection &in, const Projection &out,
  qreal deviceRatio, bool hidpi, bool hillShading, int style, int layer)
{
	Q_UNUSED(in);
	Q_UNUSED(out);
	Q_UNUSED(layer);

	_mapRatio = hidpi ? deviceRatio : 1.0;
	_zooms = _zoomsBase;

	if (_mvt) {
		_tileRatio = deviceRatio;
		_style = (style >= 0 && style < Style::styles().size())
		  ? Style::styles().at(style) : defaultStyle();

		for (int i = _zooms.last().base + 1; i <= ZOOMS.max(); i++) {
			Zoom z(i, _zooms.last().base);
			if (_tileSize * _tileRatio * (1U<<(z.z - z.base)) > MAX_TILE_SIZE)
				break;
			_zooms.append(Zoom(i, _zooms.last().base));
		}

		QPixmapCache::clear();
	}

	_coordinatesRatio = _mapRatio > 1.0 ? _mapRatio / _tileRatio : 1.0;
	_factor = zoom2scale(_zooms.at(_zoom).z, _tileSize) * _coordinatesRatio;

	_hillShading = PMTilesMap::hillShading() & hillShading;

	if (!_file.open(QIODevice::ReadOnly))
		qWarning("%s: %s", qUtf8Printable(_file.fileName()),
		  qUtf8Printable(_file.errorString()));
}

void PMTilesMap::unload()
{
	cancelJobs(true);
	_file.close();
	_cache.clear();
}

QString PMTilesMap::name() const
{
	return _name.isEmpty() ? Map::name() : _name;
}

QRectF PMTilesMap::bounds()
{
	return QRectF(ll2xy(_bounds.topLeft()), ll2xy(_bounds.bottomRight()));
}

qreal PMTilesMap::resolution(const QRectF &rect)
{
	return OSM::resolution(rect.center(), _zooms.at(_zoom).z, tileSize());
}

int PMTilesMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (!rect.isValid())
		_zoom = _zooms.size() - 1;
	else {
		QRectF tbr(ll2m(rect.topLeft()), ll2m(rect.bottomRight()));
		QPointF sc(tbr.width() / size.width(), tbr.height() / size.height());
		int zoom = scale2zoom(qMax(sc.x(), -sc.y()) / _coordinatesRatio,
		  _tileSize);

		_zoom = 0;
		for (int i = 1; i < _zooms.size(); i++) {
			if (_zooms.at(i).z > zoom)
				break;
			_zoom = i;
		}
	}

	_factor = zoom2scale(_zooms.at(_zoom).z, _tileSize) * _coordinatesRatio;

	return _zoom;
}

void PMTilesMap::setZoom(int zoom)
{
	_zoom = zoom;
	_factor = zoom2scale(_zooms.at(_zoom).z, _tileSize) * _coordinatesRatio;
}

int PMTilesMap::zoomIn()
{
	cancelJobs(false);

	_zoom = qMin(_zoom + 1, _zooms.size() - 1);
	_factor = zoom2scale(_zooms.at(_zoom).z, _tileSize) * _coordinatesRatio;

	return _zoom;
}

int PMTilesMap::zoomOut()
{
	cancelJobs(false);

	_zoom = qMax(_zoom - 1, 0);
	_factor = zoom2scale(_zooms.at(_zoom).z, _tileSize) * _coordinatesRatio;

	return _zoom;
}

qreal PMTilesMap::tileSize() const
{
	return (_tileSize / _coordinatesRatio);
}

QByteArray PMTilesMap::tileData(quint64 id)
{
	const Directory *d = findDir(_root, id);
	if (!d)
		return QByteArray();
	if (!d->runLength) {
		QVector<Directory> *leaf = _cache.object(d->offset);
		if (!leaf) {
			leaf = new QVector<Directory>(readDir(_file, _leafOffset + d->offset,
			  d->length, _ic));
			_cache.insert(d->offset, leaf);
		}
		const Directory *l = findDir(*leaf, id);
		return (l)
		  ? readData(_file, _tileOffset + l->offset, l->length, 1)
		  : QByteArray();
	} else
		return readData(_file, _tileOffset + d->offset, d->length, 1);
}

QString PMTilesMap::key(int zoom, const QPoint &xy) const
{
	return path() + "-" + QString::number(zoom) + "_"
	  + QString::number(xy.x()) + "_" + QString::number(xy.y());
}

bool PMTilesMap::isRunning(int zoom, const QPoint &xy) const
{
	for (int i = 0; i < _jobs.size(); i++) {
		const QList<RasterTile> &tiles = _jobs.at(i)->tiles();
		for (int j = 0; j < tiles.size(); j++) {
			const RasterTile &mt = tiles.at(j);
			if (mt.zoom() == zoom && mt.xy() == xy)
				return true;
		}
	}

	return false;
}

void PMTilesMap::runJob(MVTJob *job)
{
	_jobs.append(job);

	connect(job, &MVTJob::finished, this, &PMTilesMap::jobFinished);
	job->run();
}

void PMTilesMap::removeJob(MVTJob *job)
{
	_jobs.removeOne(job);
	job->deleteLater();
}

void PMTilesMap::jobFinished(MVTJob *job)
{
	const QList<RasterTile> &tiles = job->tiles();

	for (int i = 0; i < tiles.size(); i++) {
		const RasterTile &mt = tiles.at(i);
		if (!mt.pixmap().isNull())
			QPixmapCache::insert(key(mt.zoom(), mt.xy()), mt.pixmap());
	}

	removeJob(job);

	emit tilesLoaded();
}

void PMTilesMap::cancelJobs(bool wait)
{
	for (int i = 0; i < _jobs.size(); i++)
		_jobs.at(i)->cancel(wait);
}

QPointF PMTilesMap::tilePos(const QPointF &tl, const QPoint &tc,
  const QPoint &tile, unsigned overzoom) const
{
	return QPointF(tl.x() + ((tc.x() - tile.x()) << overzoom) * tileSize(),
	  tl.y() + ((tc.y() - tile.y()) << overzoom) * tileSize());
}

void PMTilesMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	const Zoom &zoom = _zooms.at(_zoom);
	unsigned overzoom = zoom.z - zoom.base;
	QPoint tile = mercator2tile(QPointF(rect.topLeft().x(),
	  -rect.topLeft().y()) * _factor, zoom.base);
	QPointF tl(tile2mercator(tile, zoom.base) / _factor);
	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	unsigned f = 1U<<overzoom;
	int width = ceil(s.width() / (tileSize() * f));
	int height = ceil(s.height() / (tileSize() * f));

	QList<RasterTile> tiles;

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			QPoint t(tile.x() + i, tile.y() + j);

			if (isRunning(zoom.z, t))
				continue;

			QPixmap pm;
			if (QPixmapCache::find(key(zoom.z, t), &pm)) {
				QPointF tp(tilePos(tl, t, tile, overzoom));
				drawTile(painter, pm, tp);
			} else
				tiles.append(RasterTile(Source(tileData(id(zoom.base, t)),
				  _tc == 2, _mvt), _style, zoom.z, t, _tileSize, _tileRatio,
				  overzoom, _hillShading));
		}
	}

	if (!tiles.isEmpty()) {
		if (flags & Map::Block || !_mvt) {
			QFuture<void> future = QtConcurrent::map(tiles, &RasterTile::render);
			future.waitForFinished();

			for (int i = 0; i < tiles.size(); i++) {
				const RasterTile &mt = tiles.at(i);
				QPixmap pm(mt.pixmap());
				if (pm.isNull())
					continue;

				QPixmapCache::insert(key(mt.zoom(), mt.xy()), pm);

				QPointF tp(tilePos(tl, mt.xy(), tile, overzoom));
				drawTile(painter, pm, tp);
			}
		} else
			runJob(new MVTJob(tiles));
	}
}

void PMTilesMap::drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp)
{
	pixmap.setDevicePixelRatio(_mapRatio > 1.0 ? _mapRatio : _tileRatio);
	painter->drawPixmap(tp, pixmap);
}

QPointF PMTilesMap::ll2xy(const Coordinates &c)
{
	QPointF m = ll2m(c);
	return QPointF(m.x(), -m.y()) / _factor;
}

Coordinates PMTilesMap::xy2ll(const QPointF &p)
{
	return m2ll(QPointF(p.x(), -p.y()) * _factor);
}

const Style *PMTilesMap::defaultStyle() const
{
	for (int i = 0; i < Style::styles().size(); i++)
		if (Style::styles().at(i)->matches(_layers))
			return Style::styles().at(i);

	qWarning("%s: no matching MVT style found", qUtf8Printable(path()));

	return Style::styles().isEmpty() ? 0 : Style::styles().first();
}

bool PMTilesMap::hillShading() const
{
	return _style && _style->hasHillShading();
}

QStringList PMTilesMap::styles(int &defaultStyle) const
{
	QStringList list;

	if (_mvt) {
		list.reserve(Style::styles().size());
		for (int i = 0; i < Style::styles().size(); i++)
			list.append(Style::styles().at(i)->name());

		defaultStyle = Style::styles().indexOf(_style);
	} else
		defaultStyle = -1;

	return list;
}

Map *PMTilesMap::create(const QString &path, const Projection &proj, bool *isDir)
{
	Q_UNUSED(proj);

	if (isDir)
		*isDir = false;

	return new PMTilesMap(path);
}
