#include <QPixmapCache>
#include <QPainter>
#include <QImageReader>
#include "osm.h"
#include "coros5map.h"

#define MVT_TILE_SIZE   512
#define MAX_TILE_SIZE   4096
#define ROOT_CACHE_SIZE 16
#define ANY_ID          0xFFFFFFFF

using namespace PMTiles;
using namespace MVT;

Coros5Map::MapTile::MapTile(const QString &path) : path(path)
{
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly)) {
		qWarning("%s: %s", qUtf8Printable(path),
		  qUtf8Printable(file.errorString()));
		return;
	}

	Header hdr;
	QString err;
	if (!readHeader(file, hdr, err)) {
		qWarning("%s: %s", qUtf8Printable(path), qUtf8Printable(err));
		return;
	}

	bounds = RectC(pos(hdr.minLon, hdr.maxLat), pos(hdr.maxLon, hdr.minLat));
	if (!bounds.isValid()) {
		qWarning("%s: invalid map bounds", qUtf8Printable(path));
		return;
	}
	zooms = Range(hdr.minZ, hdr.maxZ);
	if (!zooms.isValid()) {
		qWarning("%s: invalid map zooms", qUtf8Printable(path));
		return;
	}

	rootOffset = hdr.rootOffset;
	rootLength = hdr.rootLength;
	tileOffset = hdr.tileOffset;
	leafOffset = hdr.leafOffset;
	metadataOffset = hdr.metadataOffset;
	metadataLength = hdr.metadataLength;
	tc = hdr.tc;
	ic = hdr.ic;
	tt = hdr.tt;
}

QStringList Coros5Map::MapTile::vectorLayers() const
{
	QStringList layers;

	if (!metadataLength) {
		qWarning("%s: missing metadata", qUtf8Printable(path));
		return layers;
	}

	QFile file(path);
	if (!file.open(QIODevice::ReadOnly)) {
		qWarning("%s: %s", qUtf8Printable(path),
		  qUtf8Printable(file.errorString()));
		return layers;
	}

	QByteArray uba(readData(file, metadataOffset, metadataLength, ic));
	if (uba.isNull()) {
		qWarning("%s: error reading metadata", qUtf8Printable(path));
		return layers;
	}

	QJsonParseError error;
	QJsonDocument doc(QJsonDocument::fromJson(uba, &error));
	if (doc.isNull()) {
		qWarning("%s: metadata error: %s", qUtf8Printable(path),
		  qUtf8Printable(error.errorString()));
		return layers;
	}

	QJsonObject json(doc.object());
	QJsonArray vl(json["vector_layers"].toArray());
	for (int i = 0; i < vl.size(); i++)
		layers.append(vl.at(i).toObject()["id"].toString());

	return layers;
}

void Coros5Map::loadDir(const QString &path, Range &zooms)
{
	QDir md(path);
	md.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
	QFileInfoList ml = md.entryInfoList();
	double min[2], max[2];

	for (int i = 0; i < ml.size(); i++) {
		const QFileInfo &fi = ml.at(i);

		if (fi.isDir())
			loadDir(fi.absoluteFilePath(), zooms);
		else {
			MapTile *map = new MapTile(fi.absoluteFilePath());
			if (map->isValid()) {
				min[0] = map->bounds.left();
				min[1] = map->bounds.bottom();
				max[0] = map->bounds.right();
				max[1] = map->bounds.top();

				_maps.Insert(min, max, map);
				_bounds |= map->bounds;
				zooms |= map->zooms;
			} else
				delete map;
		}
	}
}

Coros5Map::Coros5Map(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _style(0), _mapRatio(1.0), _tileRatio(1.0),
  _mvt(false), _valid(false)
{
	QFileInfo fi(fileName);
	QDir dir(fi.absolutePath());
	Range zooms;

	QDir mapDir(dir.filePath("map"));
	if (!mapDir.exists()) {
		_errorString = "Map directory not found";
		return;
	}

	// tiles tree
	QDir vsmDir(mapDir.filePath("VSM"));
	loadDir(vsmDir.absolutePath(), zooms);
	if (!_maps.Count()) {
		_errorString = "No usable MBTiles map found";
		return;
	}

	// zooms
	for (int i = zooms.min(); i <= zooms.max(); i++)
		_zoomsBase.append(Zoom(i, i));
	if (_zoomsBase.isEmpty()) {
		_errorString = "Invalid zoom levels range";
		return;
	}
	_zoom = _zoomsBase.size() - 1;

	// tile info
	MapTree::Iterator it;
	_maps.GetFirst(it);
	const MapTile *mt = _maps.GetAt(it);

	if (mt->tt == 1) {
		_layers = mt->vectorLayers();
		_tileSize = MVT_TILE_SIZE;
		_mvt = true;
	} else {
		QByteArray data((mt->tc == 2)
		  ? Util::gunzip(tileData(mt, ANY_ID)) : tileData(mt, ANY_ID));
		QBuffer buffer(&data);
		QImageReader reader(&buffer);
		QSize tileSize(reader.size());
		if (!tileSize.isValid() || tileSize.width() != tileSize.height()) {
			_errorString = "Unsupported tile images";
			return;
		}
		_tileSize = tileSize.width();
	}

	_cache.setMaxCost(ROOT_CACHE_SIZE);

	_valid = true;
}

Coros5Map::~Coros5Map()
{
	MapTree::Iterator it;
	for (_maps.GetFirst(it); !_maps.IsNull(it); _maps.GetNext(it))
		delete _maps.GetAt(it);
}

void Coros5Map::load(const Projection &in, const Projection &out,
  qreal deviceRatio, bool hidpi, int style, int layer)
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

		for (int i = _zooms.last().base + 1; i <= OSM::ZOOMS.max(); i++) {
			Zoom z(i, _zooms.last().base);
			if (_tileSize * _tileRatio * (1U<<(z.z - z.base)) > MAX_TILE_SIZE)
				break;
			_zooms.append(Zoom(i, _zooms.last().base));
		}

		QPixmapCache::clear();
	}
}

void Coros5Map::unload()
{
	cancelJobs(true);
	_cache.clear();
}

QRectF Coros5Map::bounds()
{
	return QRectF(ll2xy(_bounds.topLeft()), ll2xy(_bounds.bottomRight()));
}

int Coros5Map::zoomFit(const QSize &size, const RectC &rect)
{
	if (!rect.isValid())
		_zoom = _zooms.size() - 1;
	else {
		QRectF tbr(OSM::ll2m(rect.topLeft()), OSM::ll2m(rect.bottomRight()));
		QPointF sc(tbr.width() / size.width(), tbr.height() / size.height());
		int zoom = OSM::scale2zoom(qMax(sc.x(), -sc.y()) / coordinatesRatio(),
		  _tileSize);

		_zoom = 0;
		for (int i = 1; i < _zooms.size(); i++) {
			if (_zooms.at(i).z > zoom)
				break;
			_zoom = i;
		}
	}

	return _zoom;
}

qreal Coros5Map::resolution(const QRectF &rect)
{
	return OSM::resolution(rect.center(), _zooms.at(_zoom).z, tileSize());
}

int Coros5Map::zoomIn()
{
	cancelJobs(false);

	_zoom = qMin(_zoom + 1, _zooms.size() - 1);
	return _zoom;
}

int Coros5Map::zoomOut()
{
	cancelJobs(false);

	_zoom = qMax(_zoom - 1, 0);
	return _zoom;
}

qreal Coros5Map::coordinatesRatio() const
{
	return _mapRatio > 1.0 ? _mapRatio / _tileRatio : 1.0;
}

qreal Coros5Map::imageRatio() const
{
	return _mapRatio > 1.0 ? _mapRatio : _tileRatio;
}

qreal Coros5Map::tileSize() const
{
	return (_tileSize / coordinatesRatio());
}

QString Coros5Map::key(int zoom, const QPoint &xy) const
{
	return path() + "-" + QString::number(zoom) + "_"
	  + QString::number(xy.x()) + "_" + QString::number(xy.y());
}

bool Coros5Map::isRunning(int zoom, const QPoint &xy) const
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

void Coros5Map::runJob(MVTJob *job)
{
	_jobs.append(job);

	connect(job, &MVTJob::finished, this, &Coros5Map::jobFinished);
	job->run();
}

void Coros5Map::removeJob(MVTJob *job)
{
	_jobs.removeOne(job);
	job->deleteLater();
}

void Coros5Map::jobFinished(MVTJob *job)
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

void Coros5Map::cancelJobs(bool wait)
{
	for (int i = 0; i < _jobs.size(); i++)
		_jobs.at(i)->cancel(wait);
}

QPointF Coros5Map::tilePos(const QPointF &tl, const QPoint &tc,
  const QPoint &tile, unsigned overzoom) const
{
	return QPointF(tl.x() + ((tc.x() - tile.x()) << overzoom) * tileSize(),
	  tl.y() + ((tc.y() - tile.y()) << overzoom) * tileSize());
}

bool Coros5Map::cb(MapTile *data, void *context)
{
	MapTile **tile = (MapTile**)context;
	*tile = data;
	return false;
}

void Coros5Map::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	const Zoom &zoom = _zooms.at(_zoom);
	unsigned overzoom = zoom.z - zoom.base;
	qreal scale = OSM::zoom2scale(zoom.base, _tileSize << overzoom);
	QPoint tile = OSM::mercator2tile(QPointF(rect.topLeft().x() * scale,
	  -rect.topLeft().y() * scale) * coordinatesRatio(), zoom.base);
	QPointF tlm(OSM::tile2mercator(tile, zoom.base));
	QPointF tl(QPointF(tlm.x() / scale, tlm.y() / scale) / coordinatesRatio());
	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	unsigned f = 1U<<overzoom;
	int width = ceil(s.width() / (tileSize() * f));
	int height = ceil(s.height() / (tileSize() * f));
	double min[2], max[2];
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
			} else {
				MapTile *map = 0;
				Coordinates tl(OSM::tile2ll(t, zoom.base));
				Coordinates br(OSM::tile2ll(QPoint(t.x() + 1, t.y() + 1),
				  zoom.base));
				RectC r(Coordinates(tl.lon(), -tl.lat()),
				  Coordinates(br.lon(), -br.lat()));

				min[0] = r.left();
				min[1] = r.bottom();
				max[0] = r.right();
				max[1] = r.top();
				_maps.Search(min, max, cb, &map);

				if (map)
					tiles.append(RasterTile(tileData(map, id(zoom.base, t)),
					  _mvt, map->tc == 2, _style, zoom.z,
					  QRect(t, QSize(_tileSize, _tileSize)), _tileRatio,
					  overzoom));
			}
		}
	}

	if (!tiles.isEmpty()) {
		if (flags & Map::Block) {
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

QByteArray Coros5Map::tileData(const MapTile *map, quint64 id)
{
	CacheEntry *ce = _cache.object(map);
	if (!ce) {
		ce = new CacheEntry(map);
		_cache.insert(map, ce);
	}

	if (id == ANY_ID) {
		if (ce->root.isEmpty())
			return QByteArray();
		id = ce->root.first().tileId;
	}

	const Directory *d = findDir(ce->root, id);
	if (!d)
		return QByteArray();
	if (!d->runLength) {
		QVector<Directory> leaf(readDir(ce->file, map->leafOffset + d->offset,
		  d->length, map->ic));
		const Directory *l = findDir(leaf, id);
		return (l)
		  ? readData(ce->file, map->tileOffset + l->offset, l->length, 1)
		  : QByteArray();
	} else
		return readData(ce->file, map->tileOffset + d->offset, d->length, 1);
}

void Coros5Map::drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp)
{
	pixmap.setDevicePixelRatio(imageRatio());
	painter->drawPixmap(tp, pixmap);
}

QPointF Coros5Map::ll2xy(const Coordinates &c)
{
	qreal scale = OSM::zoom2scale(_zooms.at(_zoom).z, _tileSize);
	QPointF m = OSM::ll2m(c);
	return QPointF(m.x() / scale, m.y() / -scale) / coordinatesRatio();
}

Coordinates Coros5Map::xy2ll(const QPointF &p)
{
	qreal scale = OSM::zoom2scale(_zooms.at(_zoom).z, _tileSize);
	return OSM::m2ll(QPointF(p.x() * scale, -p.y() * scale)
	  * coordinatesRatio());
}

const Style *Coros5Map::defaultStyle() const
{
	for (int i = 0; i < Style::styles().size(); i++)
		if (Style::styles().at(i)->matches(_layers))
			return Style::styles().at(i);

	qWarning("%s: no matching MVT style found", qUtf8Printable(path()));

	return 0;
}

QStringList Coros5Map::styles(int &defaultStyle) const
{
	QStringList list;

	if (_mvt) {
		list.reserve(Style::styles().size());
		for (int i = 0; i < Style::styles().size(); i++)
			list.append(Style::styles().at(i)->name());

		defaultStyle = Style::styles().indexOf(_style);
		if (defaultStyle < 0)
			defaultStyle = 0;
	} else
		defaultStyle = -1;

	return list;
}

Map *Coros5Map::create(const QString &path, const Projection &proj, bool *isDir)
{
	Q_UNUSED(proj);

	if (isDir)
		*isDir = true;

	return new Coros5Map(path);
}
