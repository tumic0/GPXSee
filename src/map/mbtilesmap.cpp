#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QPainter>
#include <QPixmapCache>
#include <QImageReader>
#include <QBuffer>
#include <QtConcurrent>
#include "common/util.h"
#include "osm.h"
#include "mbtilesmap.h"


#define META_TYPE(type) static_cast<QMetaType::Type>(type)

class MBTile
{
public:
	MBTile(int zoom, int scaledSize, const QPoint &xy, const QByteArray &data,
	  const QString &key) : _zoom(zoom), _scaledSize(scaledSize), _xy(xy),
	  _data(data), _key(key) {}

	const QPoint &xy() const {return _xy;}
	const QString &key() const {return _key;}
	QPixmap pixmap() const {return QPixmap::fromImage(_image);}

	void load() {
		QByteArray z(QString::number(_zoom).toLatin1());

		QBuffer buffer(&_data);
		QImageReader reader(&buffer, z);
		if (_scaledSize)
			reader.setScaledSize(QSize(_scaledSize, _scaledSize));
		reader.read(&_image);
	}

private:
	int _zoom;
	int _scaledSize;
	QPoint _xy;
	QByteArray _data;
	QString _key;
	QImage _image;
};


MBTilesMap::MBTilesMap(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _mapRatio(1.0), _tileRatio(1.0), _scalable(false),
  _scaledSize(0), _valid(false)
{
	_db = QSqlDatabase::addDatabase("QSQLITE", fileName);
	_db.setDatabaseName(fileName);
	_db.setConnectOptions("QSQLITE_OPEN_READONLY");

	if (!_db.open()) {
		_errorString = "Error opening database file";
		return;
	}

	QSqlRecord r = _db.record("tiles");
	if (r.isEmpty()
	  || r.field(0).name() != "zoom_level"
	  || META_TYPE(r.field(0).type()) != QMetaType::Int
	  || r.field(1).name() != "tile_column"
	  || META_TYPE(r.field(1).type()) != QMetaType::Int
	  || r.field(2).name() != "tile_row"
	  || META_TYPE(r.field(2).type()) != QMetaType::Int
	  || r.field(3).name() != "tile_data"
	  || META_TYPE(r.field(3).type()) != QMetaType::QByteArray) {
		_errorString = "Invalid table format";
		return;
	}

	{
		QSqlQuery query("SELECT DISTINCT zoom_level FROM tiles"
		  " ORDER BY zoom_level", _db);
		while (query.next())
			_zooms.append(query.value(0).toInt());
		if (_zooms.isEmpty()) {
			_errorString = "Empty tile set";
			return;
		}
		if (_zooms.first() < 0) {
			_errorString = "Invalid zoom levels";
			return;
		}
	}
	_zi = _zooms.size() - 1;

	{
		int z = _zooms.first();
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

	{
		QString sql = QString("SELECT tile_data FROM tiles LIMIT 1");
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

	{
		QSqlQuery query("SELECT value FROM metadata WHERE name = 'format'", _db);
		if (query.first()) {
			if (query.value(0).toString() == "pbf")
				_scalable = true;
		} else
			qWarning("%s: missing tiles format", qPrintable(fileName));
	}

	{
		QSqlQuery query("SELECT value FROM metadata WHERE name = 'name'", _db);
		if (query.first())
			_name = query.value(0).toString();
		else {
			qWarning("%s: missing map name", qPrintable(fileName));
			_name = Util::file2name(fileName);
		}
	}

	{
		QSqlQuery query(
		  "SELECT value FROM metadata WHERE name = 'tilepixelratio'", _db);
		if (query.first()) {
			bool ok;
			_tileRatio = query.value(0).toString().toDouble(&ok);
			if (!ok) {
				_errorString = "Invalid tile pixel ratio";
				return;
			}
		}
	}

	_db.close();

	_valid = true;
}

void MBTilesMap::load()
{
	_db.open();
}

void MBTilesMap::unload()
{
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
			if (_zooms.at(i) > zoom)
				break;
			_zi = i;
		}
	}

	return _zooms.at(_zi);
}

qreal MBTilesMap::resolution(const QRectF &rect)
{
	return OSM::resolution(rect.center(), _zooms.at(_zi), _tileSize);
}

int MBTilesMap::zoomIn()
{
	_zi = qMin(_zi + 1, _zooms.size() - 1);
	return _zooms.at(_zi);
}

int MBTilesMap::zoomOut()
{
	_zi = qMax(_zi - 1, 0);
	return _zooms.at(_zi);
}

void MBTilesMap::setDevicePixelRatio(qreal deviceRatio, qreal mapRatio)
{
	_mapRatio = mapRatio;

	if (_scalable) {
		_scaledSize = _tileSize * deviceRatio;
		_tileRatio = deviceRatio;
	}
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

void MBTilesMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	Q_UNUSED(flags);
	int zoom = _zooms.at(_zi);
	qreal scale = OSM::zoom2scale(zoom, _tileSize);
	QRectF b(bounds());


	QPoint tile = OSM::mercator2tile(QPointF(rect.topLeft().x() * scale,
	  -rect.topLeft().y() * scale) * coordinatesRatio(), zoom);
	QPointF tl(floor(rect.left() / tileSize())
	  * tileSize(), floor(rect.top() / tileSize()) * tileSize());

	QSizeF s(qMin(rect.right() - tl.x(), b.width()),
	  qMin(rect.bottom() - tl.y(), b.height()));
	int width = ceil(s.width() / tileSize());
	int height = ceil(s.height() / tileSize());


	QList<MBTile> tiles;

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			QPixmap pm;
			QPoint t(tile.x() + i, tile.y() + j);
			QString key = path() + "-" + QString::number(zoom) + "_"
			  + QString::number(t.x()) + "_" + QString::number(t.y());

			if (QPixmapCache::find(key, &pm)) {
				QPointF tp(qMax(tl.x(), b.left()) + (t.x() - tile.x())
				  * tileSize(), qMax(tl.y(), b.top()) + (t.y() - tile.y())
				  * tileSize());
				drawTile(painter, pm, tp);
			} else {
				tiles.append(MBTile(zoom, _scaledSize, t, tileData(zoom, t),
				  key));
			}
		}
	}

	QFuture<void> future = QtConcurrent::map(tiles, &MBTile::load);
	future.waitForFinished();

	for (int i = 0; i < tiles.size(); i++) {
		const MBTile &mt = tiles.at(i);
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

void MBTilesMap::drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp)
{
	pixmap.setDevicePixelRatio(imageRatio());
	painter->drawPixmap(tp, pixmap);
}

QPointF MBTilesMap::ll2xy(const Coordinates &c)
{
	qreal scale = OSM::zoom2scale(_zooms.at(_zi), _tileSize);
	QPointF m = OSM::ll2m(c);
	return QPointF(m.x() / scale, m.y() / -scale) / coordinatesRatio();
}

Coordinates MBTilesMap::xy2ll(const QPointF &p)
{
	qreal scale = OSM::zoom2scale(_zooms.at(_zi), _tileSize);
	return OSM::m2ll(QPointF(p.x() * scale, -p.y() * scale)
	  * coordinatesRatio());
}

Map *MBTilesMap::create(const QString &path, const Projection &, bool *isDir)
{
	if (isDir)
		*isDir = false;

	return new MBTilesMap(path);
}
