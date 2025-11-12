#include <QDataStream>
#include <QPainter>
#include <QPixmapCache>
#include <QtConcurrent>
#include "osm.h"
#include "tile.h"
#include "gemfmap.h"

using namespace OSM;

static bool readSources(QDataStream &stream)
{
	qint32 num, len, idx;
	QByteArray name;

	stream >> num;

	for (int i = 0; i < num; i++) {
		stream >> idx >> len;
		name.resize(len);
		stream.readRawData(name.data(), name.size());
	}

	return (stream.status() == QDataStream::Ok);
}

QRect GEMFMap::rect(const Zoom &zoom)
{
	QRect r;
	const QList<Region> &list = zoom.ranges;

	for (int i = 0; i < list.size(); i++) {
		const Region &rg = list.at(i);
		r |= QRect(QPoint(rg.minX, rg.minY), QPoint(rg.maxX, rg.maxY));
	}

	return r;
}

bool GEMFMap::readHeader(QDataStream &stream)
{
	qint32 version;

	stream >> version >> _tileSize;
	return (stream.status() == QDataStream::Ok
	  && (version >= 3 && version <= 4));
}

bool GEMFMap::readRegions(QDataStream &stream)
{
	qint32 num, idx, level;
	Region r;
	int zi;

	stream >> num;

	for (int i = 0; i < num; i++) {
		stream >> level >> r.minX >> r.maxX >> r.minY >> r.maxY >> idx
		  >> r.offset;

		if ((zi = _zooms.indexOf(level)) < 0) {
			zi = _zooms.size();
			_zooms.append(Zoom(level));
		}

		_zooms[zi].ranges.append(r);
	}

	return (stream.status() == QDataStream::Ok);
}

bool GEMFMap::computeBounds()
{
	if (_zooms.isEmpty())
		return false;

	std::sort(_zooms.begin(), _zooms.end());

	const Zoom &z = _zooms.first();
	QRect r(rect(z));
	if (!r.isValid())
		return false;

	Coordinates tl(tile2ll(r.topLeft(), z.level));
	tl.rlat() = -tl.lat();
	Coordinates br(tile2ll(QPoint(r.right() + 1, r.bottom() + 1), z.level));
	br.rlat() = -br.lat();
	// Workaround of broken zoom levels 0 and 1 due to numerical
	// instability
	tl.rlat() = qMin(tl.lat(), BOUNDS.top());
	br.rlat() = qMax(br.lat(), BOUNDS.bottom());

	_bounds = RectC(tl, br);

	return true;
}

GEMFMap::GEMFMap(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _file(fileName), _zi(0), _mapRatio(1.0),
  _valid(false)
{
	if (!_file.open(QIODevice::ReadOnly)) {
		_errorString = _file.errorString();
		return;
	}

	QDataStream stream(&_file);

	if (!readHeader(stream)) {
		_errorString = "Invalid/unsupported GEMF file";
		return;
	}
	if (!readSources(stream)) {
		_errorString = "Error reading tile sources";
		return;
	}
	if (!readRegions(stream)) {
		_errorString = "Error reading tile ranges";
		return;
	}
	if (!computeBounds()) {
		_errorString = "Invalid map area";
		return;
	}

	_file.close();

	_valid = true;
}

void GEMFMap::load(const Projection &in, const Projection &out,
  qreal deviceRatio, bool hidpi, bool hillShading, int style, int layer)
{
	Q_UNUSED(in);
	Q_UNUSED(out);
	Q_UNUSED(hillShading);
	Q_UNUSED(style);
	Q_UNUSED(layer);

	_mapRatio = hidpi ? deviceRatio : 1.0;
	_factor = zoom2scale(_zooms.at(_zi).level, _tileSize) * _mapRatio;

	if (!_file.open(QIODevice::ReadOnly))
		qWarning("%s: %s", qUtf8Printable(_file.fileName()),
		  qUtf8Printable(_file.errorString()));
}

void GEMFMap::unload()
{
	_file.close();
}

qreal GEMFMap::resolution(const QRectF &rect)
{
	return OSM::resolution(rect.center(), _zooms.at(_zi).level, tileSize());
}

QRectF GEMFMap::bounds()
{
	return QRectF(ll2xy(_bounds.topLeft()), ll2xy(_bounds.bottomRight()));
}

int GEMFMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (!rect.isValid())
		_zi = _zooms.size() - 1;
	else {
		QRectF tbr(ll2m(rect.topLeft()), ll2m(rect.bottomRight()));
		QPointF sc(tbr.width() / size.width(), tbr.height() / size.height());
		int zoom = scale2zoom(qMax(sc.x(), -sc.y()) / _mapRatio, _tileSize);

		_zi = 0;
		for (int i = 1; i < _zooms.size(); i++) {
			if (_zooms.at(i).level > zoom)
				break;
			_zi = i;
		}
	}

	_factor = zoom2scale(_zooms.at(_zi).level, _tileSize) * _mapRatio;

	return _zi;
}

void GEMFMap::setZoom(int zoom)
{
	_zi = zoom;
	_factor = zoom2scale(_zooms.at(_zi).level, _tileSize) * _mapRatio;
}

int GEMFMap::zoomIn()
{
	_zi = qMin(_zi + 1, _zooms.size() - 1);
	_factor = zoom2scale(_zooms.at(_zi).level, _tileSize) * _mapRatio;

	return _zi;
}

int GEMFMap::zoomOut()
{
	_zi = qMax(_zi - 1, 0);
	_factor = zoom2scale(_zooms.at(_zi).level, _tileSize) * _mapRatio;

	return _zi;
}

QPointF GEMFMap::ll2xy(const Coordinates &c)
{
	QPointF m = ll2m(c);
	return QPointF(m.x(), -m.y()) / _factor;
}

Coordinates GEMFMap::xy2ll(const QPointF &p)
{
	return m2ll(QPointF(p.x(), -p.y()) * _factor);
}

qreal GEMFMap::tileSize() const
{
	return (_tileSize / _mapRatio);
}

QByteArray GEMFMap::tileData(const QPoint &tile)
{
	const Zoom &z = _zooms.at(_zi);

	for(int i = 0; i < z.ranges.size(); i++) {
		const Region &r = z.ranges.at(i);
		QRect rect(QPoint(r.minX, r.minY), QPoint(r.maxX, r.maxY));

		if (rect.contains(tile)) {
			quint32 x = tile.x() - r.minX;
			quint32 y = tile.y() - r.minY;
			quint32 idx = x * (r.maxY + 1 - r.minY) + y;
			quint64 offset = idx * 12;
			quint64 address;
			quint32 size;

			if (!_file.seek(offset + r.offset))
				return QByteArray();
			QDataStream stream(&_file);
			stream >> address >> size;
			if (stream.status() != QDataStream::Ok)
				return QByteArray();

			if (!_file.seek(address))
				return QByteArray();

			QByteArray data;
			data.resize(size);
			return (_file.read(data.data(), data.size()) == size)
			  ? data : QByteArray();
		}
	}

	return QByteArray();
}

void GEMFMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	Q_UNUSED(flags);
	const Zoom &z = _zooms.at(_zi);
	QPoint tile = mercator2tile(QPointF(rect.topLeft().x(),
	  -rect.topLeft().y()) * _factor, z.level);
	QPointF tl(tile2mercator(tile, z.level) / _factor);
	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	int width = ceil(s.width() / tileSize());
	int height = ceil(s.height() / tileSize());

	QList<DataTile> tiles;

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			QPixmap pm;
			QPoint t(tile.x() + i, tile.y() + j);
			QString key = path() + "-" + QString::number(z.level) + "_"
			  + QString::number(t.x()) + "_" + QString::number(t.y());

			if (QPixmapCache::find(key, &pm)) {
				QPointF tp(tl.x() + (t.x() - tile.x()) * tileSize(),
				  tl.y() + (t.y() - tile.y()) * tileSize());
				drawTile(painter, pm, tp);
			} else
				tiles.append(DataTile(t, tileData(t), key));
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
		  tl.y() + (mt.xy().y() - tile.y())* tileSize());
		drawTile(painter, pm, tp);
	}
}

void GEMFMap::drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp)
{
	pixmap.setDevicePixelRatio(_mapRatio);
	painter->drawPixmap(tp, pixmap);
}

Map *GEMFMap::create(const QString &path, const Projection &proj, bool *isDir)
{
	Q_UNUSED(proj);

	if (isDir)
		*isDir = false;

	return new GEMFMap(path);
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const GEMFMap::Region &region)
{
	dbg.nospace() << "Region(" << QRect(QPoint(region.minX, region.minY),
	  QPoint(region.maxX, region.maxY)) << ", " << region.offset << ")";

	return dbg.space();
}

QDebug operator<<(QDebug dbg, const GEMFMap::Zoom &zoom)
{
	dbg.nospace() << "Zoom(" << zoom.level << ", " << zoom.ranges << ")";

	return dbg.space();
}
#endif // QT_NO_DEBUG
