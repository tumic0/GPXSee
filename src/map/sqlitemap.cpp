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
#include "metatype.h"
#include "sqlitemap.h"


SqliteMap::SqliteMap(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _mapRatio(1.0), _valid(false)
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
	  || r.field(0).name() != "x"
	  || METATYPE(r.field(0)) != QMetaType::Int
	  || r.field(1).name() != "y"
	  || METATYPE(r.field(1)) != QMetaType::Int
	  || r.field(2).name() != "z"
	  || METATYPE(r.field(2)) != QMetaType::Int
	  || r.field(4).name() != "image"
	  || METATYPE(r.field(4)) != QMetaType::QByteArray) {
		_errorString = "Invalid table format";
		return;
	}

	{
		QSqlQuery query("SELECT min(z), max(z) FROM tiles", _db);
		if (!query.first()) {
			_errorString = "Empty tile set";
			return;
		}
		_zooms = Range(17 - query.value(1).toInt(), 17 - query.value(0).toInt());
		if (_zooms.min() < 0 || !_zooms.isValid()) {
			_errorString = "Invalid zoom levels";
			return;
		}
	}
	_zoom = _zooms.max();

	{
		int z = _zooms.min();
		QString sql = QString("SELECT min(x), min(y), max(x), max(y) FROM tiles"
		  " WHERE z = %1").arg(17 - z);
		QSqlQuery query(sql, _db);
		query.first();

		int minX = qMin((1<<z) - 1, qMax(0, query.value(0).toInt()));
		int minY = qMin((1<<z) - 1, qMax(0, query.value(1).toInt()));
		int maxX = qMin((1<<z) - 1, qMax(0, query.value(2).toInt())) + 1;
		int maxY = qMin((1<<z) - 1, qMax(0, query.value(3).toInt())) + 1;
		Coordinates tl(OSM::tile2ll(QPoint(minX, minY), z));
		tl.rlat() = -tl.lat();
		Coordinates br(OSM::tile2ll(QPoint(maxX, maxY), z));
		br.rlat() = -br.lat();
		// Workaround of broken zoom levels 0 and 1 due to numerical instability
		tl.rlat() = qMin(tl.lat(), OSM::BOUNDS.top());
		br.rlat() = qMax(br.lat(), OSM::BOUNDS.bottom());
		_bounds = RectC(tl, br);
	}

	{
		QString sql = QString("SELECT image FROM tiles LIMIT 1");
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

void SqliteMap::load(const Projection &in, const Projection &out,
  qreal deviceRatio, bool hidpi)
{
	Q_UNUSED(in);
	Q_UNUSED(out);

	_mapRatio = hidpi ? deviceRatio : 1.0;
	_db.open();
}

void SqliteMap::unload()
{
	_db.close();
}

QRectF SqliteMap::bounds()
{
	return QRectF(ll2xy(_bounds.topLeft()), ll2xy(_bounds.bottomRight()));
}

int SqliteMap::limitZoom(int zoom) const
{
	if (zoom < _zooms.min())
		return _zooms.min();
	if (zoom > _zooms.max())
		return _zooms.max();

	return zoom;
}

int SqliteMap::zoomFit(const QSize &size, const RectC &rect)
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

qreal SqliteMap::resolution(const QRectF &rect)
{
	return OSM::resolution(rect.center(), _zoom, tileSize());
}

int SqliteMap::zoomIn()
{
	_zoom = qMin(_zoom + 1, _zooms.max());
	return _zoom;
}

int SqliteMap::zoomOut()
{
	_zoom = qMax(_zoom - 1, _zooms.min());
	return _zoom;
}

qreal SqliteMap::tileSize() const
{
	return (_tileSize / _mapRatio);
}

QByteArray SqliteMap::tileData(int zoom, const QPoint &tile) const
{
	QSqlQuery query(_db);
	query.prepare("SELECT image FROM tiles WHERE z=:zoom AND x=:x AND y=:y");
	query.bindValue(":zoom", 17 - zoom);
	query.bindValue(":x", tile.x());
	query.bindValue(":y", tile.y());
	query.exec();

	if (query.first())
		return query.value(0).toByteArray();

	return QByteArray();
}

void SqliteMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	Q_UNUSED(flags);
	qreal scale = OSM::zoom2scale(_zoom, _tileSize);
	QPoint tile = OSM::mercator2tile(QPointF(rect.topLeft().x() * scale,
	  -rect.topLeft().y() * scale) * _mapRatio, _zoom);
	QPointF tlm(OSM::tile2mercator(tile, _zoom));
	QPointF tl(QPointF(tlm.x() / scale, tlm.y() / scale) / _mapRatio);
	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	int width = ceil(s.width() / tileSize());
	int height = ceil(s.height() / tileSize());

	QList<DataTile> tiles;

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			QPixmap pm;
			QPoint t(tile.x() + i, tile.y() + j);
			QString key = path() + "-" + QString::number(_zoom) + "_"
			  + QString::number(t.x()) + "_" + QString::number(t.y());

			if (QPixmapCache::find(key, &pm)) {
				QPointF tp(tl.x() + (t.x() - tile.x()) * tileSize(),
				  tl.y() + (t.y() - tile.y()) * tileSize());
				drawTile(painter, pm, tp);
			} else
				tiles.append(DataTile(t, tileData(_zoom, t), key));
		}
	}

	QFuture<void> future = QtConcurrent::map(tiles, &DataTile::load);
	future.waitForFinished();

	for (int i = 0; i < tiles.size(); i++) {
		const DataTile &mt = tiles.at(i);
		QPixmap pm(mt.pixmap());
		if (pm.isNull())
			continue;

		QPixmapCache::insert(mt.key(), pm);

		QPointF tp(tl.x() + (mt.xy().x() - tile.x()) * tileSize(),
		  tl.y() + (mt.xy().y() - tile.y()) * tileSize());
		drawTile(painter, pm, tp);
	}
}

void SqliteMap::drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp)
{
	pixmap.setDevicePixelRatio(_mapRatio);
	painter->drawPixmap(tp, pixmap);
}

QPointF SqliteMap::ll2xy(const Coordinates &c)
{
	qreal scale = OSM::zoom2scale(_zoom, _tileSize);
	QPointF m = OSM::ll2m(c);
	return QPointF(m.x() / scale, m.y() / -scale) / _mapRatio;
}

Coordinates SqliteMap::xy2ll(const QPointF &p)
{
	qreal scale = OSM::zoom2scale(_zoom, _tileSize);
	return OSM::m2ll(QPointF(p.x() * scale, -p.y() * scale) * _mapRatio);
}

Map *SqliteMap::create(const QString &path, const Projection &proj, bool *isDir)
{
	Q_UNUSED(proj);

	if (isDir)
		*isDir = false;

	return new SqliteMap(path);
}
