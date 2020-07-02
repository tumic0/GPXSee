#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QFileInfo>
#include <QPainter>
#include <QPixmapCache>
#include <QImageReader>
#include <QBuffer>
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QtCore>
#else // QT_VERSION < 5
#include <QtConcurrent>
#endif // QT_VERSION < 5
#include "common/rectc.h"
#include "common/config.h"
#include "osm.h"
#include "mbtilesmap.h"


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

#define META_TYPE(type) static_cast<QMetaType::Type>(type)

static double index2mercator(int index, int zoom)
{
	return rad2deg(-M_PI + 2 * M_PI * ((double)index / (1<<zoom)));
}

MBTilesMap::MBTilesMap(const QString &fileName, QObject *parent)
  : Map(parent), _fileName(fileName), _mapRatio(1.0), _tileRatio(1.0),
  _scalable(false), _scaledSize(0), _valid(false)
{
	_db = QSqlDatabase::addDatabase("QSQLITE", fileName);
	_db.setDatabaseName(fileName);
	_db.setConnectOptions("QSQLITE_OPEN_READONLY");

	if (!_db.open()) {
		_errorString = fileName + ": Error opening database file";
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
		QSqlQuery query("SELECT min(zoom_level), max(zoom_level) FROM tiles",
		  _db);
		if (!query.first()) {
			_errorString = "Empty tile set";
			return;
		}
		_zooms = Range(query.value(0).toInt(), query.value(1).toInt());
		if (_zooms.min() < 0 || !_zooms.isValid()) {
			_errorString = "Invalid zoom levels";
			return;
		}
	}
	_zoom = _zooms.max();

	{
		QString sql = QString("SELECT min(tile_column), min(tile_row), "
		  "max(tile_column), max(tile_row) FROM tiles WHERE zoom_level = %1")
		  .arg(_zooms.min());
		QSqlQuery query(sql, _db);
		query.first();

		double minX = index2mercator(qMin((1<<_zooms.min()) - 1,
		  qMax(0, query.value(0).toInt())), _zooms.min());
		double minY = index2mercator(qMin((1<<_zooms.min()) - 1,
		  qMax(0, query.value(1).toInt())), _zooms.min());
		double maxX = index2mercator(qMin((1<<_zooms.min()) - 1,
		  qMax(0, query.value(2).toInt())) + 1, _zooms.min());
		double maxY = index2mercator(qMin((1<<_zooms.min()) - 1,
		  qMax(0, query.value(3).toInt())) + 1, _zooms.min());
		Coordinates tl(OSM::m2ll(QPointF(minX, maxY)));
		Coordinates br(OSM::m2ll(QPointF(maxX, minY)));
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
			qWarning("%s: missing tiles format", qPrintable(_fileName));
	}

	{
		QSqlQuery query("SELECT value FROM metadata WHERE name = 'name'", _db);
		if (query.first())
			_name = query.value(0).toString();
		else {
			qWarning("%s: missing map name", qPrintable(_fileName));
			_name = QFileInfo(_fileName).fileName();
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

int MBTilesMap::limitZoom(int zoom) const
{
	if (zoom < _zooms.min())
		return _zooms.min();
	if (zoom > _zooms.max())
		return _zooms.max();

	return zoom;
}

int MBTilesMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (!rect.isValid())
		_zoom = _zooms.max();
	else {
		QRectF tbr(OSM::ll2m(rect.topLeft()), OSM::ll2m(rect.bottomRight()));
		QPointF sc(tbr.width() / size.width(), tbr.height() / size.height());
		_zoom = limitZoom(OSM::scale2zoom(qMax(sc.x(), -sc.y())
		  / coordinatesRatio(), _tileSize));
	}

	return _zoom;
}

qreal MBTilesMap::resolution(const QRectF &rect)
{
	return OSM::resolution(rect.center(), _zoom, _tileSize);
}

int MBTilesMap::zoomIn()
{
	_zoom = qMin(_zoom + 1, _zooms.max());
	return _zoom;
}

int MBTilesMap::zoomOut()
{
	_zoom = qMax(_zoom - 1, _zooms.min());
	return _zoom;
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
	qreal scale = OSM::zoom2scale(_zoom, _tileSize);
	QRectF b(bounds());


	QPoint tile = OSM::mercator2tile(QPointF(rect.topLeft().x() * scale,
	  -rect.topLeft().y() * scale) * coordinatesRatio(), _zoom);
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
			QString key = _fileName + "-" + QString::number(_zoom) + "_"
			  + QString::number(t.x()) + "_" + QString::number(t.y());

			if (QPixmapCache::find(key, pm)) {
				QPointF tp(qMax(tl.x(), b.left()) + (t.x() - tile.x())
				  * tileSize(), qMax(tl.y(), b.top()) + (t.y() - tile.y())
				  * tileSize());
				drawTile(painter, pm, tp);
			} else {
				tiles.append(MBTile(_zoom, _scaledSize, t, tileData(_zoom, t),
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
#ifdef ENABLE_HIDPI
	pixmap.setDevicePixelRatio(imageRatio());
#endif // ENABLE_HIDPI
	painter->drawPixmap(tp, pixmap);
}

QPointF MBTilesMap::ll2xy(const Coordinates &c)
{
	qreal scale = OSM::zoom2scale(_zoom, _tileSize);
	QPointF m = OSM::ll2m(c);
	return QPointF(m.x() / scale, m.y() / -scale) / coordinatesRatio();
}

Coordinates MBTilesMap::xy2ll(const QPointF &p)
{
	qreal scale = OSM::zoom2scale(_zoom, _tileSize);
	return OSM::m2ll(QPointF(p.x() * scale, -p.y() * scale)
	  * coordinatesRatio());
}
