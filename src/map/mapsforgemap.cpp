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
	cancelJobs();

	_zoom = qMin(_zoom + 1, _data.zooms().max());
	updateTransform();
	return _zoom;
}

int MapsforgeMap::zoomOut()
{
	cancelJobs();

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

QString MapsforgeMap::key(int zoom, const QPoint &xy) const
{
	return path() + "-" + QString::number(zoom) + "_"
	  + QString::number(xy.x()) + "_" + QString::number(xy.y());
}

bool MapsforgeMap::isRunning(int zoom, const QPoint &xy) const
{
	for (int i = 0; i < _jobs.size(); i++) {
		const QList<Mapsforge::RasterTile> &tiles = _jobs.at(i)->tiles();
		for (int j = 0; j < tiles.size(); j++) {
			const Mapsforge::RasterTile &mt = tiles.at(j);
			if (mt.zoom() == zoom && mt.xy() == xy)
				return true;
		}
	}

	return false;
}

void MapsforgeMap::runJob(MapsforgeMapJob *job)
{
	_jobs.append(job);

	connect(job, &MapsforgeMapJob::finished, this, &MapsforgeMap::jobFinished);
	job->run();
}

void MapsforgeMap::removeJob(MapsforgeMapJob *job)
{
	_jobs.removeOne(job);
	job->deleteLater();
}

void MapsforgeMap::jobFinished(MapsforgeMapJob *job)
{
	const QList<Mapsforge::RasterTile> &tiles = job->tiles();

	for (int i = 0; i < tiles.size(); i++) {
		const Mapsforge::RasterTile &mt = tiles.at(i);
		if (mt.isValid())
			QPixmapCache::insert(key(mt.zoom(), mt.xy()), mt.pixmap());
	}

	removeJob(job);

	emit tilesLoaded();
}

void MapsforgeMap::cancelJobs()
{
	for (int i = 0; i < _jobs.size(); i++)
		_jobs.at(i)->cancel();
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
			QPoint ttl(tl.x() + i * _data.tileSize(), tl.y() + j
			  * _data.tileSize());
			if (isRunning(_zoom, ttl))
				continue;

			QPixmap pm;
			if (QPixmapCache::find(key(_zoom, ttl), &pm))
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
				  _tileRatio, paths, points));
			}
		}
	}

	if (!tiles.isEmpty())
		runJob(new MapsforgeMapJob(tiles));
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
