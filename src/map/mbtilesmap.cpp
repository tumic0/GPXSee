#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QPainter>
#include <QPixmapCache>
#include <QtConcurrent>
#include "common/util.h"
#include "osm.h"
#include "metatype.h"
#include "mbtilesmap.h"

#define MAX_TILE_SIZE 4096

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
		qWarning("%s: missing minzoom metadata", qPrintable(path()));
		zoom = OSM::ZOOMS.min();
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
		qWarning("%s: missing maxzoom metadata", qPrintable(path()));
		zoom = OSM::ZOOMS.max();
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

	_zi = _zoomsBase.size() - 1;

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
		qWarning("%s: missing bounds metadata", qPrintable(path()));

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
		Coordinates tl(OSM::tile2ll(QPoint(minX, maxY), z));
		Coordinates br(OSM::tile2ll(QPoint(maxX, minY), z));
		// Workaround of broken zoom levels 0 and 1 due to numerical instability
		tl.rlat() = qMin(tl.lat(), OSM::BOUNDS.top());
		br.rlat() = qMax(br.lat(), OSM::BOUNDS.bottom());
		_bounds = RectC(tl, br);
	}

	return true;
}

bool MBTilesMap::getTileSize()
{
	QString sql("SELECT zoom_level, tile_data FROM tiles LIMIT 1");
	QSqlQuery query(sql, _db);
	query.first();

	QByteArray z(QByteArray::number(query.value(0).toInt()));
	QByteArray data = query.value(1).toByteArray();
	QBuffer buffer(&data);
	QImageReader reader(&buffer, z);
	QSize tileSize(reader.size());

	if (!tileSize.isValid() || tileSize.width() != tileSize.height()) {
		_errorString = "Unsupported/invalid tile images";
		return false;
	}

	_tileSize = tileSize.width();

	return true;
}

void MBTilesMap::getTileFormat()
{
	QSqlQuery query("SELECT value FROM metadata WHERE name = 'format'", _db);
	if (query.first()) {
		if (query.value(0).toString() == "pbf")
			_scalable = true;
	} else
		qWarning("%s: missing tiles format metadata", qPrintable(path()));
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
		qWarning("%s: missing map name", qPrintable(path()));
		_name = Util::file2name(path());
	}
}

MBTilesMap::MBTilesMap(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _mapRatio(1.0), _tileRatio(1.0), _scalable(false),
  _scaledSize(0), _valid(false)
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
	if (!getTileSize())
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
  qreal deviceRatio, bool hidpi)
{
	Q_UNUSED(in);
	Q_UNUSED(out);

	_mapRatio = hidpi ? deviceRatio : 1.0;
	_zooms = _zoomsBase;

	if (_scalable) {
		_scaledSize = _tileSize * deviceRatio;
		_tileRatio = deviceRatio;

		for (int i = _zooms.last().base + 1; i <= OSM::ZOOMS.max(); i++) {
			Zoom z(i, _zooms.last().base);
			if (_tileSize * _tileRatio * (1U<<(z.z - z.base)) > MAX_TILE_SIZE)
				break;
			_zooms.append(Zoom(i, _zooms.last().base));
		}
	}

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

int MBTilesMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (!rect.isValid())
		_zi = _zooms.size() - 1;
	else {
		QRectF tbr(OSM::ll2m(rect.topLeft()), OSM::ll2m(rect.bottomRight()));
		QPointF sc(tbr.width() / size.width(), tbr.height() / size.height());
		int zoom = OSM::scale2zoom(qMax(sc.x(), -sc.y()) / coordinatesRatio(),
		  _tileSize);

		_zi = 0;
		for (int i = 1; i < _zooms.size(); i++) {
			if (_zooms.at(i).z > zoom)
				break;
			_zi = i;
		}
	}

	return _zi;
}

qreal MBTilesMap::resolution(const QRectF &rect)
{
	return OSM::resolution(rect.center(), _zooms.at(_zi).z, tileSize());
}

int MBTilesMap::zoomIn()
{
	cancelJobs(false);

	_zi = qMin(_zi + 1, _zooms.size() - 1);
	return _zi;
}

int MBTilesMap::zoomOut()
{
	cancelJobs(false);

	_zi = qMax(_zi - 1, 0);
	return _zi;
}

qreal MBTilesMap::coordinatesRatio() const
{
	return _mapRatio > 1.0 ? _mapRatio / _tileRatio : 1.0;
}

qreal MBTilesMap::imageRatio() const
{
	return _mapRatio > 1.0 ? _mapRatio : _tileRatio;
}

qreal MBTilesMap::tileSize() const
{
	return (_tileSize / coordinatesRatio());
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

bool MBTilesMap::isRunning(const QString &key) const
{
	for (int i = 0; i < _jobs.size(); i++) {
		const QList<MBTile> &tiles = _jobs.at(i)->tiles();
		for (int j = 0; j < tiles.size(); j++)
			if (tiles.at(j).key() == key)
				return true;
	}

	return false;
}

void MBTilesMap::runJob(MBTilesMapJob *job)
{
	_jobs.append(job);

	connect(job, &MBTilesMapJob::finished, this, &MBTilesMap::jobFinished);
	job->run();
}

void MBTilesMap::removeJob(MBTilesMapJob *job)
{
	_jobs.removeOne(job);
	job->deleteLater();
}

void MBTilesMap::jobFinished(MBTilesMapJob *job)
{
	const QList<MBTile> &tiles = job->tiles();

	for (int i = 0; i < tiles.size(); i++) {
		const MBTile &mt = tiles.at(i);
		if (!mt.pixmap().isNull())
			QPixmapCache::insert(mt.key(), mt.pixmap());
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
	const Zoom &zoom = _zooms.at(_zi);
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

	QList<MBTile> tiles;

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			QPixmap pm;
			QPoint t(tile.x() + i, tile.y() + j);
			QString key = path() + "-" + QString::number(zoom.z) + "_"
			  + QString::number(t.x()) + "_" + QString::number(t.y());

			if (isRunning(key))
				continue;

			if (QPixmapCache::find(key, &pm)) {
				QPointF tp(tilePos(tl, t, tile, overzoom));
				drawTile(painter, pm, tp);
			} else
				tiles.append(MBTile(zoom.z, overzoom, _scaledSize, t,
				  tileData(zoom.base, t), key));
		}
	}

	if (!tiles.isEmpty()) {
		if (flags & Map::Block || !_scalable) {
			QFuture<void> future = QtConcurrent::map(tiles, &MBTile::load);
			future.waitForFinished();

			for (int i = 0; i < tiles.size(); i++) {
				const MBTile &mt = tiles.at(i);
				QPixmap pm(mt.pixmap());
				if (pm.isNull())
					continue;

				QPixmapCache::insert(mt.key(), pm);

				QPointF tp(tilePos(tl, mt.xy(), tile, overzoom));
				drawTile(painter, pm, tp);
			}
		} else
			runJob(new MBTilesMapJob(tiles));
	}
}

void MBTilesMap::drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp)
{
	pixmap.setDevicePixelRatio(imageRatio());
	painter->drawPixmap(tp, pixmap);
}

QPointF MBTilesMap::ll2xy(const Coordinates &c)
{
	qreal scale = OSM::zoom2scale(_zooms.at(_zi).z, _tileSize);
	QPointF m = OSM::ll2m(c);
	return QPointF(m.x() / scale, m.y() / -scale) / coordinatesRatio();
}

Coordinates MBTilesMap::xy2ll(const QPointF &p)
{
	qreal scale = OSM::zoom2scale(_zooms.at(_zi).z, _tileSize);
	return OSM::m2ll(QPointF(p.x() * scale, -p.y() * scale)
	  * coordinatesRatio());
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
