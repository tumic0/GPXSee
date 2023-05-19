#include <QPainter>
#include <QPixmapCache>
#include "common/range.h"
#include "common/wgs84.h"
#include "rectd.h"
#include "pcs.h"
#include "encmap.h"


using namespace ENC;

#define TILE_SIZE 512

ENCMap::ENCMap(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _data(fileName), _projection(PCS::pcs(3857)),
  _tileRatio(1.0), _zoom(0)
{
	if (_data.isValid()) {
		_llBounds = _data.bounds();
		updateTransform();
	}
}

void ENCMap::load(const Projection &in, const Projection &out,
  qreal deviceRatio, bool hidpi)
{
	Q_UNUSED(in);
	Q_UNUSED(hidpi);

	_tileRatio = deviceRatio;
	_projection = out;
	_data.load();
	QPixmapCache::clear();
}

void ENCMap::unload()
{
	cancelJobs(true);
	_data.clear();
}

int ENCMap::zoomFit(const QSize &size, const RectC &rect)
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

int ENCMap::zoomIn()
{
	cancelJobs(false);

	_zoom = qMin(_zoom + 1, _data.zooms().max());
	updateTransform();
	return _zoom;
}

int ENCMap::zoomOut()
{
	cancelJobs(false);

	_zoom = qMax(_zoom - 1, _data.zooms().min());
	updateTransform();
	return _zoom;
}

void ENCMap::setZoom(int zoom)
{
	_zoom = zoom;
	updateTransform();
}

Transform ENCMap::transform(int zoom) const
{
	int z = zoom + Util::log2i(TILE_SIZE);

	double scale = _projection.isGeographic()
	  ? 360.0 / (1<<z) : (2.0 * M_PI * WGS84_RADIUS) / (1<<z);
	PointD topLeft(_projection.ll2xy(_llBounds.topLeft()));
	return Transform(ReferencePoint(PointD(0, 0), topLeft),
	  PointD(scale, scale));
}

void ENCMap::updateTransform()
{
	_transform = transform(_zoom);

	RectD prect(_llBounds, _projection);
	_bounds = QRectF(_transform.proj2img(prect.topLeft()),
	  _transform.proj2img(prect.bottomRight()));
}

bool ENCMap::isRunning(int zoom, const QPoint &xy) const
{
	for (int i = 0; i < _jobs.size(); i++) {
		const QList<ENC::RasterTile> &tiles = _jobs.at(i)->tiles();
		for (int j = 0; j < tiles.size(); j++) {
			const ENC::RasterTile &mt = tiles.at(j);
			if (mt.zoom() == zoom && mt.xy() == xy)
				return true;
		}
	}

	return false;
}

void ENCMap::runJob(ENCMapJob *job)
{
	_jobs.append(job);

	connect(job, &ENCMapJob::finished, this, &ENCMap::jobFinished);
	job->run();
}

void ENCMap::removeJob(ENCMapJob *job)
{
	_jobs.removeOne(job);
	job->deleteLater();
}

void ENCMap::jobFinished(ENCMapJob *job)
{
	const QList<ENC::RasterTile> &tiles = job->tiles();

	for (int i = 0; i < tiles.size(); i++) {
		const ENC::RasterTile &mt = tiles.at(i);
		if (mt.isValid())
			QPixmapCache::insert(key(mt.zoom(), mt.xy()), mt.pixmap());
	}

	removeJob(job);

	emit tilesLoaded();
}

void ENCMap::cancelJobs(bool wait)
{
	for (int i = 0; i < _jobs.size(); i++)
		_jobs.at(i)->cancel(wait);
}

QString ENCMap::key(int zoom, const QPoint &xy) const
{
	return path() + "-" + QString::number(zoom) + "_"
	  + QString::number(xy.x()) + "_" + QString::number(xy.y());
}

void ENCMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	Q_UNUSED(flags);
	QPointF tl(floor(rect.left() / TILE_SIZE) * TILE_SIZE,
	  floor(rect.top() / TILE_SIZE) * TILE_SIZE);
	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	int width = ceil(s.width() / TILE_SIZE);
	int height = ceil(s.height() / TILE_SIZE);

	QList<RasterTile> tiles;

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			QPoint ttl(tl.x() + i * TILE_SIZE, tl.y() + j * TILE_SIZE);
			if (isRunning(_zoom, ttl))
				continue;

			QPixmap pm;
			if (QPixmapCache::find(key(_zoom, ttl), &pm))
				painter->drawPixmap(ttl, pm);
			else
				tiles.append(RasterTile(_projection, _transform, &_data,
				  _zoom, QRect(ttl, QSize(TILE_SIZE, TILE_SIZE)), _tileRatio));
		}
	}

	if (!tiles.isEmpty()) {
		if (flags & Map::Block) {
			QFuture<void> future = QtConcurrent::map(tiles, &RasterTile::render);
			future.waitForFinished();

			for (int i = 0; i < tiles.size(); i++) {
				const RasterTile &mt = tiles.at(i);
				const QPixmap &pm = mt.pixmap();
				painter->drawPixmap(mt.xy(), pm);
				QPixmapCache::insert(key(mt.zoom(), mt.xy()), pm);
			}
		} else
			runJob(new ENCMapJob(tiles));
	}
}

Map *ENCMap::create(const QString &path, bool *isMap)
{
	if (isMap)
		*isMap = false;

	return new ENCMap(path);
}
