#include <QPainter>
#include "common/wgs84.h"
#include "pcs.h"
#include "rectd.h"
#include "mapsforgemap.h"


using namespace Mapsforge;

#define TEXT_EXTENT 160

static int log2i(unsigned val)
{
	int ret = 0;

	while (val >>= 1)
		ret++;

	return ret;
}

MapsforgeMap::MapsforgeMap(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _data(fileName), _zoom(0),
  _projection(PCS::pcs(3857)), _tileRatio(1.0)
{
	_zoom = _data.zooms().min();
	updateTransform();
}

void MapsforgeMap::load()
{
	_data.load();
}

void MapsforgeMap::unload()
{
	_data.clear();
}

int MapsforgeMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (rect.isValid()) {
		RectD pr(rect, _projection, 10);

		_zoom = _data.zooms().min();
		for (int i = _data.zooms().min() + 1; i <= _data.zooms().max(); i++) {
			Transform t(transform(i));
			QRectF r(t.proj2img(pr.topLeft()), t.proj2img(pr.bottomRight()));
			if (size.width() < r.width() || size.height() < r.height())
				break;
			_zoom = i;
		}
	} else
		_zoom = _data.zooms().max();

	updateTransform();

	return _zoom;
}

int MapsforgeMap::zoomIn()
{
	_zoom = qMin(_zoom + 1, _data.zooms().max());
	updateTransform();
	return _zoom;
}

int MapsforgeMap::zoomOut()
{
	_zoom = qMax(_zoom - 1, _data.zooms().min());
	updateTransform();
	return _zoom;
}

void MapsforgeMap::setZoom(int zoom)
{
	_zoom = zoom;
	updateTransform();
}

Transform MapsforgeMap::transform(int zoom) const
{
	int z = zoom + log2i(_data.tileSize());

	double scale = _projection.isGeographic()
	  ? 360.0 / (1<<z) : (2.0 * M_PI * WGS84_RADIUS) / (1<<z);
	PointD topLeft(_projection.ll2xy(_data.bounds().topLeft()));
	return Transform(ReferencePoint(PointD(0, 0), topLeft),
	  PointD(scale, scale));
}

void MapsforgeMap::updateTransform()
{
	_transform = transform(_zoom);

	RectD prect(_data.bounds(), _projection);
	_bounds = QRectF(_transform.proj2img(prect.topLeft()),
	  _transform.proj2img(prect.bottomRight()));
	// Adjust the bounds of world maps to avoid problems with wrapping
	if (_data.bounds().left() <= -180.0 || _data.bounds().right() >= 180.0)
		_bounds.adjust(0.5, 0, -0.5, 0);
}

bool MapsforgeMap::isRunning(const QString &key) const
{
	return _running.contains(key);
}

void MapsforgeMap::addRunning(const QList<RasterTile> &tiles)
{
	for (int i = 0; i < tiles.size(); i++)
		_running.insert(tiles.at(i).key());
}

void MapsforgeMap::removeRunning(const QList<RasterTile> &tiles)
{
	for (int i = 0; i < tiles.size(); i++)
		_running.remove(tiles.at(i).key());
}

void MapsforgeMap::jobFinished(const QList<RasterTile> &tiles)
{
	removeRunning(tiles);
	emit tilesLoaded();
}

void MapsforgeMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	Q_UNUSED(flags);

	QPointF tl(floor(rect.left() / _data.tileSize()) * _data.tileSize(),
	  floor(rect.top() / _data.tileSize()) * _data.tileSize());
	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	int width = ceil(s.width() / _data.tileSize());
	int height = ceil(s.height() / _data.tileSize());

	QList<RasterTile> tiles;

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			QPixmap pm;
			QPoint ttl(tl.x() + i * _data.tileSize(), tl.y() + j
			  * _data.tileSize());
			QString key = path() + "-" + QString::number(_zoom) + "_"
			  + QString::number(ttl.x()) + "_" + QString::number(ttl.y());

			if (isRunning(key))
				continue;

			if (QPixmapCache::find(key, &pm))
				painter->drawPixmap(ttl, pm);
			else {
				QList<MapData::Path> paths;
				QList<MapData::Point> points;

				/* Add a "sub-pixel" margin to assure the tile areas do not
				   overlap on the border lines. This prevents areas overlap
				   artifacts at least when using the EPSG:3857 projection. */
				QRectF pathRect(QPointF(ttl.x() + 0.5, ttl.y() + 0.5),
				  QPointF(ttl.x() + _data.tileSize() - 0.5, ttl.y()
				  + _data.tileSize() - 0.5));
				pathRect &= _bounds;
				RectD pathRectD(_transform.img2proj(pathRect.topLeft()),
				  _transform.img2proj(pathRect.bottomRight()));
				_data.paths(pathRectD.toRectC(_projection, 20), _zoom, &paths);

				QRectF pointRect(QPointF(ttl.x() - TEXT_EXTENT, ttl.y()
				  - TEXT_EXTENT), QPointF(ttl.x() + _data.tileSize()
				  + TEXT_EXTENT, ttl.y() + _data.tileSize() + TEXT_EXTENT));
				pointRect &= _bounds;
				RectD pointRectD(_transform.img2proj(pointRect.topLeft()),
				  _transform.img2proj(pointRect.bottomRight()));
				_data.points(pointRectD.toRectC(_projection, 20), _zoom,
				  &points);

				tiles.append(RasterTile(_projection, _transform, _zoom,
				  QRect(ttl, QSize(_data.tileSize(), _data.tileSize())),
				  _tileRatio, key, paths, points));
			}
		}
	}

	if (!tiles.isEmpty()) {
		MapsforgeMapJob *job = new MapsforgeMapJob(tiles);
		connect(job, &MapsforgeMapJob::finished, this,
		  &MapsforgeMap::jobFinished);
		addRunning(tiles);
		job->run();
	}
}

void MapsforgeMap::setDevicePixelRatio(qreal deviceRatio, qreal mapRatio)
{
	Q_UNUSED(mapRatio);

	_tileRatio = deviceRatio;
}

void MapsforgeMap::setOutputProjection(const Projection &projection)
{
	if (projection == _projection)
		return;

	_projection = projection;
	updateTransform();
	QPixmapCache::clear();
}
