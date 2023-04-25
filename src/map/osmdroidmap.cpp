#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QPainter>
#include <QPixmapCache>
#include <QImageReader>
#include <QBuffer>
#include <QtConcurrent>
#include "osm.h"
#include "tile.h"
#include "osmdroidmap.h"


#define META_TYPE(type) static_cast<QMetaType::Type>(type)

OsmdroidMap::OsmdroidMap(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _mapRatio(1.0), _valid(false)
{
	quint64 z, l = 0, r = 0, t = 0, b = 0;

	if (!Util::isSQLiteDB(fileName, _errorString))
		return;

	_db = QSqlDatabase::addDatabase("QSQLITE", fileName);
	_db.setDatabaseName(fileName);
	_db.setConnectOptions("QSQLITE_OPEN_READONLY");

	if (!_db.open()) {
		_errorString = _db.lastError().text();
		return;
	}

	QSqlRecord rcrd = _db.record("tiles");
	if (rcrd.isEmpty()
	  || rcrd.field(0).name() != "key"
	  || META_TYPE(rcrd.field(0).type()) != QMetaType::Int
	  || rcrd.field(1).name() != "provider"
	  || META_TYPE(rcrd.field(1).type()) != QMetaType::QString
	  || rcrd.field(2).name() != "tile"
	  || META_TYPE(rcrd.field(2).type()) != QMetaType::QByteArray) {
		_errorString = "Invalid table format";
		return;
	}

	{
		QSqlQuery query("SELECT min(key), max(key) FROM tiles", _db);
		if (!query.first()) {
			_errorString = "Empty tile set";
			return;
		}

		quint64 min = query.value(0).toLongLong();
		quint64 max = query.value(1).toLongLong();

		for (quint64 i = 0; i < (quint64)OSM::ZOOMS.max(); i++) {
			quint64 key = ((i << i) << i);

			if (key <= min)
				_zooms.setMin(i);
			if (key <= max)
				_zooms.setMax(i);
		}


	}

	_zoom = _zooms.max();
	z = _zooms.min();

	{
		quint64 minz = ((z << z) << z);
		quint64 maxz = (((z + 1) << (z + 1)) << (z + 1));

		QSqlQuery query(_db);
		query.prepare("SELECT min(key), max(key) FROM tiles"
		  " where key >= :min AND key < :max");
		query.bindValue(":min", minz);
		query.bindValue(":max", maxz);
		query.exec();
		if (!query.first())
			return;

		quint64 min = query.value(0).toLongLong();
		quint64 max = query.value(1).toLongLong();

		for (quint64 i = 0; i < 1ull<<z; i++) {
			quint64 key = (((z << z) + i) << z);

			if (key <= min)
				l = i;
			if (key <= max)
				r = i;
		}
	}

	{
		quint64 minx = (((z << z) + l) << z);
		quint64 maxx = (((z << z) + l + 1) << z);

		QSqlQuery query(_db);
		query.prepare("SELECT min(key), max(key) FROM tiles"
		  " where key >= :min AND key < :max");
		query.bindValue(":min", minx);
		query.bindValue(":max", maxx);
		query.exec();
		if (!query.first())
			return;

		quint64 min = query.value(0).toLongLong();
		quint64 max = query.value(1).toLongLong();

		for (quint64 i = 0; i < 1ull<<z; i++) {
			quint64 key = (((z << z) + l) << z) + i;

			if (key <= min)
				t = i;
			if (key <= max)
				b = i;
		}
	}

	Coordinates tl(OSM::tile2ll(QPoint(l, t), z));
	tl.rlat() = -tl.lat();
	Coordinates br(OSM::tile2ll(QPoint(r + 1, b + 1), z));
	br.rlat() = -br.lat();
	// Workaround of broken zoom levels 0 and 1 due to numerical
	// instability
	tl.rlat() = qMin(tl.lat(), OSM::BOUNDS.top());
	br.rlat() = qMax(br.lat(), OSM::BOUNDS.bottom());
	_bounds = RectC(tl, br);

	{
		QString sql = QString("SELECT tile FROM tiles LIMIT 1");
		QSqlQuery query(sql, _db);
		query.first();

		QByteArray data = query.value(0).toByteArray();
		QBuffer buffer(&data);
		QImageReader reader(&buffer);
		QSize tileSize(reader.size());

		if (!tileSize.isValid() || tileSize.width() != tileSize.height()) {
			_errorString = "Unsupported/invalid tile images";
			return;
		}
		_tileSize = tileSize.width();
	}

	_db.close();

	_valid = true;
}

void OsmdroidMap::load()
{
	_db.open();
}

void OsmdroidMap::unload()
{
	_db.close();
}

QRectF OsmdroidMap::bounds()
{
	return QRectF(ll2xy(_bounds.topLeft()), ll2xy(_bounds.bottomRight()));
}

int OsmdroidMap::limitZoom(int zoom) const
{
	if (zoom < _zooms.min())
		return _zooms.min();
	if (zoom > _zooms.max())
		return _zooms.max();

	return zoom;
}

int OsmdroidMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (!rect.isValid())
		_zoom = _zooms.max();
	else {
		QRectF tbr(OSM::ll2m(rect.topLeft()), OSM::ll2m(rect.bottomRight()));
		QPointF sc(tbr.width() / size.width(), tbr.height() / size.height());
		_zoom = limitZoom(OSM::scale2zoom(qMax(sc.x(), -sc.y()) / _mapRatio,
		  _tileSize));
	}

	return _zoom;
}

qreal OsmdroidMap::resolution(const QRectF &rect)
{
	return OSM::resolution(rect.center(), _zoom, _tileSize);
}

int OsmdroidMap::zoomIn()
{
	_zoom = qMin(_zoom + 1, _zooms.max());
	return _zoom;
}

int OsmdroidMap::zoomOut()
{
	_zoom = qMax(_zoom - 1, _zooms.min());
	return _zoom;
}

void OsmdroidMap::setDevicePixelRatio(qreal deviceRatio, qreal mapRatio)
{
	Q_UNUSED(deviceRatio);
	_mapRatio = mapRatio;
}

qreal OsmdroidMap::tileSize() const
{
	return (_tileSize / _mapRatio);
}

QByteArray OsmdroidMap::tileData(int zoom, const QPoint &tile) const
{
	quint64 z = zoom;
	quint64 key = (((z << z) + tile.x()) << z) + tile.y();

	QSqlQuery query(_db);
	query.prepare("SELECT tile FROM tiles WHERE key=:key");
	query.bindValue(":key", key);
	query.exec();

	if (query.first())
		return query.value(0).toByteArray();

	return QByteArray();
}

void OsmdroidMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	Q_UNUSED(flags);
	qreal scale = OSM::zoom2scale(_zoom, _tileSize);
	QRectF b(bounds());


	QPoint tile = OSM::mercator2tile(QPointF(rect.topLeft().x() * scale,
	  -rect.topLeft().y() * scale) * _mapRatio, _zoom);
	QPointF tl(floor(rect.left() / tileSize())
	  * tileSize(), floor(rect.top() / tileSize()) * tileSize());

	QSizeF s(qMin(rect.right() - tl.x(), b.width()),
	  qMin(rect.bottom() - tl.y(), b.height()));
	int width = ceil(s.width() / tileSize());
	int height = ceil(s.height() / tileSize());


	QList<RenderTile> tiles;

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			QPixmap pm;
			QPoint t(tile.x() + i, tile.y() + j);
			QString key = path() + "-" + QString::number(_zoom) + "_"
			  + QString::number(t.x()) + "_" + QString::number(t.y());

			if (QPixmapCache::find(key, &pm)) {
				QPointF tp(qMax(tl.x(), b.left()) + (t.x() - tile.x())
				  * tileSize(), qMax(tl.y(), b.top()) + (t.y() - tile.y())
				  * tileSize());
				drawTile(painter, pm, tp);
			} else {
				tiles.append(RenderTile(t, tileData(_zoom, t), key));
			}
		}
	}

	QFuture<void> future = QtConcurrent::map(tiles, &RenderTile::load);
	future.waitForFinished();

	for (int i = 0; i < tiles.size(); i++) {
		const RenderTile &mt = tiles.at(i);
		QPixmap pm(mt.pixmap());
		if (pm.isNull())
			continue;

		QPixmapCache::insert(mt.key(), pm);

		QPointF tp(qMax(tl.x(), b.left()) + (mt.xy().x() - tile.x())
		  * tileSize(), qMax(tl.y(), b.top()) + (mt.xy().y() - tile.y())
		  * tileSize());
		drawTile(painter, pm, tp);
	}
}

void OsmdroidMap::drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp)
{
	pixmap.setDevicePixelRatio(_mapRatio);
	painter->drawPixmap(tp, pixmap);
}

QPointF OsmdroidMap::ll2xy(const Coordinates &c)
{
	qreal scale = OSM::zoom2scale(_zoom, _tileSize);
	QPointF m = OSM::ll2m(c);
	return QPointF(m.x() / scale, m.y() / -scale) / _mapRatio;
}

Coordinates OsmdroidMap::xy2ll(const QPointF &p)
{
	qreal scale = OSM::zoom2scale(_zoom, _tileSize);
	return OSM::m2ll(QPointF(p.x() * scale, -p.y() * scale) * _mapRatio);
}

Map *OsmdroidMap::create(const QString &path, const Projection &, bool *isDir)
{
	if (isDir)
		*isDir = false;

	return new OsmdroidMap(path);
}
