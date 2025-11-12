#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QPainter>
#include <QPixmapCache>
#include <QImageReader>
#include "common/util.h"
#include "osm.h"
#include "metatype.h"
#include "mbtilesmap.h"

#define MVT_TILE_SIZE 512
#define MAX_TILE_SIZE 4096

using namespace MVT;
using namespace OSM;

static RectC str2bounds(const QString &str)
{
	QStringList list(str.split(','));
	if (list.size() != 4)
		return RectC();

	bool lok, rok, bok, tok;
	double left = list.at(0).toDouble(&lok);
	double bottom = list.at(1).toDouble(&bok);
	double right = list.at(2).toDouble(&rok);
	double top = list.at(3).toDouble(&tok);

	return (lok && rok && bok && tok)
	  ? RectC(Coordinates(left, top), Coordinates(right, bottom))
	  : RectC();
}

const Style *MBTilesMap::defaultStyle() const
{
	for (int i = 0; i < Style::styles().size(); i++)
		if (Style::styles().at(i)->matches(_layers))
			return Style::styles().at(i);

	qWarning("%s: no matching MVT style found", qUtf8Printable(path()));

	return Style::styles().isEmpty() ? 0 : Style::styles().first();
}

bool MBTilesMap::getMinZoom(int &zoom)
{
	QSqlQuery query("SELECT value FROM metadata WHERE name = 'minzoom'", _db);

	if (query.first()) {
		bool ok;
		zoom = query.value(0).toString().toInt(&ok);
		if (!ok || zoom < 0) {
			_errorString = "Invalid minzoom metadata";
			return false;
		}
	} else {
		qWarning("%s: missing minzoom metadata", qUtf8Printable(path()));
		zoom = ZOOMS.min();
	}

	return true;
}

bool MBTilesMap::getMaxZoom(int &zoom)
{
	QSqlQuery query("SELECT value FROM metadata WHERE name = 'maxzoom'", _db);

	if (query.first()) {
		bool ok;
		zoom = query.value(0).toString().toInt(&ok);
		if (!ok && zoom < 0) {
			_errorString = "Invalid maxzoom metadata";
			return false;
		}
	} else {
		qWarning("%s: missing maxzoom metadata", qUtf8Printable(path()));
		zoom = ZOOMS.max();
	}

	return true;
}

bool MBTilesMap::getZooms()
{
	int minZoom, maxZoom;

	if (!(getMinZoom(minZoom) && getMaxZoom(maxZoom)))
		return false;

	for (int i = minZoom; i <= maxZoom; i++) {
		QString sql = QString("SELECT zoom_level FROM tiles"
		  " WHERE zoom_level = %1 LIMIT 1").arg(i);
		QSqlQuery query(sql, _db);
		if (query.first())
			_zoomsBase.append(Zoom(i, i));
	}

	if (!_zoomsBase.size()) {
		_errorString = "Empty tile set";
		return false;
	}

	_zoom = _zoomsBase.size() - 1;

	return true;
}

bool MBTilesMap::getBounds()
{
	QSqlQuery query("SELECT value FROM metadata WHERE name = 'bounds'", _db);
	if (query.first()) {
		RectC b(str2bounds(query.value(0).toString()));
		if (!b.isValid()) {
			_errorString = "Invalid bounds metadata";
			return false;
		}
		_bounds = b;
	} else {
		qWarning("%s: missing bounds metadata", qUtf8Printable(path()));

		int z = _zoomsBase.first().z;
		QString sql = QString("SELECT min(tile_column), min(tile_row), "
		  "max(tile_column), max(tile_row) FROM tiles WHERE zoom_level = %1")
		  .arg(z);
		QSqlQuery query(sql, _db);
		query.first();

		int minX = qMin((1<<z) - 1, qMax(0, query.value(0).toInt()));
		int minY = qMin((1<<z) - 1, qMax(0, query.value(1).toInt()));
		int maxX = qMin((1<<z) - 1, qMax(0, query.value(2).toInt())) + 1;
		int maxY = qMin((1<<z) - 1, qMax(0, query.value(3).toInt())) + 1;
		Coordinates tl(tile2ll(QPoint(minX, maxY), z));
		Coordinates br(tile2ll(QPoint(maxX, minY), z));
		// Workaround of broken zoom levels 0 and 1 due to numerical instability
		tl.rlat() = qMin(tl.lat(), BOUNDS.top());
		br.rlat() = qMax(br.lat(), BOUNDS.bottom());
		_bounds = RectC(tl, br);
	}

	return true;
}

bool MBTilesMap::getTileSizeAndStyle()
{
	if (_mvt) {
		QSqlQuery query("SELECT value FROM metadata WHERE name = 'json'", _db);
		if (query.first()) {
			QJsonDocument doc(QJsonDocument::fromJson(
			  query.value(0).toByteArray()));
			QJsonObject json(doc.object());
			QJsonArray vl(json["vector_layers"].toArray());
			QStringList vectorLayers;
			for (int i = 0; i < vl.size(); i++)
				_layers.append(vl.at(i).toObject()["id"].toString());
		} else
			qWarning("%s: missing MVT json metadata", qUtf8Printable(path()));
		_tileSize = MVT_TILE_SIZE;
	} else {
		QString sql("SELECT tile_data FROM tiles LIMIT 1");
		QSqlQuery query(sql, _db);
		query.first();

		QByteArray data(query.value(0).toByteArray());
		QBuffer buffer(&data);
		QImageReader reader(&buffer);
		QSize tileSize(reader.size());

		if (!tileSize.isValid() || tileSize.width() != tileSize.height()) {
			_errorString = "Unsupported/invalid tile images";
			return false;
		}
		_tileSize = tileSize.width();
	}

	return true;
}

void MBTilesMap::getTileFormat()
{
	QSqlQuery query("SELECT value FROM metadata WHERE name = 'format'", _db);
	if (query.first()) {
		if (query.value(0).toString() == "pbf")
			_mvt = true;
	} else
		qWarning("%s: missing tiles format metadata", qUtf8Printable(path()));
}

void MBTilesMap::getTilePixelRatio()
{
	QSqlQuery query("SELECT value FROM metadata WHERE name = 'tilepixelratio'",
	  _db);
	if (query.first()) {
		bool ok;
		double ratio = query.value(0).toString().toDouble(&ok);
		if (ok)
			_tileRatio = ratio;
	}
}

void MBTilesMap::getName()
{
	QSqlQuery query("SELECT value FROM metadata WHERE name = 'name'", _db);
	if (query.first())
		_name = query.value(0).toString();
	else {
		qWarning("%s: missing map name", qUtf8Printable(path()));
		_name = Util::file2name(path());
	}
}

MBTilesMap::MBTilesMap(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _style(0), _mapRatio(1.0), _tileRatio(1.0),
  _hillShading(false), _mvt(false), _valid(false)
{
	if (!Util::isSQLiteDB(fileName, _errorString))
		return;

	_db = QSqlDatabase::addDatabase("QSQLITE", fileName);
	_db.setDatabaseName(fileName);
	_db.setConnectOptions("QSQLITE_OPEN_READONLY");

	if (!_db.open()) {
		_errorString = _db.lastError().text();
		return;
	}

	QSqlRecord r = _db.record("tiles");
	if (r.isEmpty()
	  || r.field(0).name() != "zoom_level"
	  || METATYPE(r.field(0)) != QMetaType::Int
	  || r.field(1).name() != "tile_column"
	  || METATYPE(r.field(1)) != QMetaType::Int
	  || r.field(2).name() != "tile_row"
	  || METATYPE(r.field(2)) != QMetaType::Int
	  || r.field(3).name() != "tile_data"
	  || METATYPE(r.field(3)) != QMetaType::QByteArray) {
		_errorString = "Invalid table format";
		return;
	}

	getTileFormat();
	if (!getTileSizeAndStyle())
		return;
	if (!getZooms())
		return;
	if (!getBounds())
		return;
	getTilePixelRatio();
	getName();

	_db.close();

	_valid = true;
}

void MBTilesMap::load(const Projection &in, const Projection &out,
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

	_hillShading = MBTilesMap::hillShading() & hillShading;

	_db.open();
}

void MBTilesMap::unload()
{
	cancelJobs(true);
	_db.close();
}

QRectF MBTilesMap::bounds()
{
	return QRectF(ll2xy(_bounds.topLeft()), ll2xy(_bounds.bottomRight()));
}

qreal MBTilesMap::resolution(const QRectF &rect)
{
	return OSM::resolution(rect.center(), _zooms.at(_zoom).z, tileSize());
}

int MBTilesMap::zoomFit(const QSize &size, const RectC &rect)
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

void MBTilesMap::setZoom(int zoom)
{
	_zoom = zoom;
	_factor = zoom2scale(_zooms.at(_zoom).z, _tileSize) * _coordinatesRatio;
}

int MBTilesMap::zoomIn()
{
	cancelJobs(false);

	_zoom = qMin(_zoom + 1, _zooms.size() - 1);
	_factor = zoom2scale(_zooms.at(_zoom).z, _tileSize) * _coordinatesRatio;

	return _zoom;
}

int MBTilesMap::zoomOut()
{
	cancelJobs(false);

	_zoom = qMax(_zoom - 1, 0);
	_factor = zoom2scale(_zooms.at(_zoom).z, _tileSize) * _coordinatesRatio;

	return _zoom;
}

qreal MBTilesMap::tileSize() const
{
	return (_tileSize / _coordinatesRatio);
}

QByteArray MBTilesMap::tileData(int zoom, const QPoint &tile) const
{
	QSqlQuery query(_db);
	query.prepare("SELECT tile_data FROM tiles "
	  "WHERE zoom_level=:zoom AND tile_column=:x AND tile_row=:y");
	query.bindValue(":zoom", zoom);
	query.bindValue(":x", tile.x());
	query.bindValue(":y", (1<<zoom) - tile.y() - 1);
	query.exec();

	if (query.first())
		return query.value(0).toByteArray();

	return QByteArray();
}

QString MBTilesMap::key(int zoom, const QPoint &xy) const
{
	return path() + "-" + QString::number(zoom) + "_"
	  + QString::number(xy.x()) + "_" + QString::number(xy.y());
}

bool MBTilesMap::isRunning(int zoom, const QPoint &xy) const
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

void MBTilesMap::runJob(MVTJob *job)
{
	_jobs.append(job);

	connect(job, &MVTJob::finished, this, &MBTilesMap::jobFinished);
	job->run();
}

void MBTilesMap::removeJob(MVTJob *job)
{
	_jobs.removeOne(job);
	job->deleteLater();
}

void MBTilesMap::jobFinished(MVTJob *job)
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

void MBTilesMap::cancelJobs(bool wait)
{
	for (int i = 0; i < _jobs.size(); i++)
		_jobs.at(i)->cancel(wait);
}

QPointF MBTilesMap::tilePos(const QPointF &tl, const QPoint &tc,
  const QPoint &tile, unsigned overzoom) const
{
	return QPointF(tl.x() + ((tc.x() - tile.x()) << overzoom) * tileSize(),
	  tl.y() + ((tc.y() - tile.y()) << overzoom) * tileSize());
}

void MBTilesMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
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
				tiles.append(RasterTile(Source(tileData(zoom.base, t), true,
				  _mvt), _style, zoom.z, t, _tileSize, _tileRatio, overzoom,
				  _hillShading));
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

void MBTilesMap::drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp)
{
	pixmap.setDevicePixelRatio(_mapRatio > 1.0 ? _mapRatio : _tileRatio);
	painter->drawPixmap(tp, pixmap);
}

QPointF MBTilesMap::ll2xy(const Coordinates &c)
{
	QPointF m = ll2m(c);
	return QPointF(m.x(), -m.y()) / _factor;
}

Coordinates MBTilesMap::xy2ll(const QPointF &p)
{
	return m2ll(QPointF(p.x(), -p.y()) * _factor);
}

bool MBTilesMap::hillShading() const
{
	return _style && _style->hasHillShading();
}

QStringList MBTilesMap::styles(int &defaultStyle) const
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

Map *MBTilesMap::create(const QString &path, const Projection &proj, bool *isDir)
{
	Q_UNUSED(proj);

	if (isDir)
		*isDir = false;

	return new MBTilesMap(path);
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const MBTilesMap::Zoom &zoom)
{
	dbg.nospace() << "Zoom(" << zoom.z << ", " << zoom.base << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
