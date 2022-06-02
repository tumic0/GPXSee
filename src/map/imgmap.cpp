#include <QFile>
#include <QPainter>
#include <QPixmapCache>
#include <QtConcurrent>
#include "common/rectc.h"
#include "common/range.h"
#include "common/wgs84.h"
#include "IMG/imgdata.h"
#include "IMG/gmapdata.h"
#include "IMG/rastertile.h"
#include "osm.h"
#include "pcs.h"
#include "rectd.h"
#include "imgmap.h"

using namespace IMG;

#define TILE_SIZE   384
#define TEXT_EXTENT 160

static QList<MapData*> overlays(const QString &fileName)
{
	QList<MapData*> list;

	for (int i = 1; i < 32; i++) {
		QString ol(fileName + "." + QString::number(i));
		if (QFileInfo(ol).isFile()) {
			MapData *data = new IMGData(ol);
			if (data->isValid())
				list.append(data);
			else {
				qWarning("%s: %s", qPrintable(data->fileName()),
				  qPrintable(data->errorString()));
				delete data;
			}
		} else
			break;
	}

	return list;
}

IMGMap::IMGMap(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _projection(PCS::pcs(3857)), _tileRatio(1.0),
  _valid(false)
{
	if (GMAPData::isGMAP(fileName))
		_data.append(new GMAPData(fileName));
	else {
		_data.append(new IMGData(fileName));
		_data.append(overlays(fileName));
	}

	if (!_data.first()->isValid()) {
		_errorString = _data.first()->errorString();
		return;
	}

	_dataBounds = _data.first()->bounds() & OSM::BOUNDS;
	_zoom = _data.first()->zooms().min();
	updateTransform();

	_valid = true;
}

void IMGMap::load()
{
	for (int i = 0; i < _data.size(); i++)
		_data.at(i)->load();
}

void IMGMap::unload()
{
	for (int i = 0; i < _data.size(); i++)
		_data.at(i)->clear();
}

int IMGMap::zoomFit(const QSize &size, const RectC &rect)
{
	const Range &zooms = _data.first()->zooms();

	if (rect.isValid()) {
		RectD pr(rect, _projection, 10);

		_zoom = zooms.min();
		for (int i = zooms.min() + 1; i <= zooms.max(); i++) {
			Transform t(transform(i));
			QRectF r(t.proj2img(pr.topLeft()), t.proj2img(pr.bottomRight()));
			if (size.width() < r.width() || size.height() < r.height())
				break;
			_zoom = i;
		}
	} else
		_zoom = zooms.max();

	updateTransform();

	return _zoom;
}

int IMGMap::zoomIn()
{
	cancelJobs();

	_zoom = qMin(_zoom + 1, _data.first()->zooms().max());
	updateTransform();
	return _zoom;
}

int IMGMap::zoomOut()
{
	cancelJobs();

	_zoom = qMax(_zoom - 1, _data.first()->zooms().min());
	updateTransform();
	return _zoom;
}

void IMGMap::setZoom(int zoom)
{
	_zoom = zoom;
	updateTransform();
}

Transform IMGMap::transform(int zoom) const
{
	double scale = _projection.isGeographic()
	  ? 360.0 / (1<<zoom) : (2.0 * M_PI * WGS84_RADIUS) / (1<<zoom);
	PointD topLeft(_projection.ll2xy(_dataBounds.topLeft()));
	return Transform(ReferencePoint(PointD(0, 0), topLeft),
	  PointD(scale, scale));
}

void IMGMap::updateTransform()
{
	_transform = transform(_zoom);

	RectD prect(_dataBounds, _projection);
	_bounds = QRectF(_transform.proj2img(prect.topLeft()),
	  _transform.proj2img(prect.bottomRight()));
	// Adjust the bounds of world maps to avoid problems with wrapping
	if (_dataBounds.left() == -180.0 || _dataBounds.right() == 180.0)
		_bounds.adjust(0.5, 0, -0.5, 0);
}

bool IMGMap::isRunning(const QString &key) const
{
	for (int i = 0; i < _jobs.size(); i++) {
		const QList<IMG::RasterTile> &tiles = _jobs.at(i)->tiles();
		for (int j = 0; j < tiles.size(); j++)
			if (tiles.at(j).key() == key)
				return true;
	}

	return false;
}

void IMGMap::runJob(IMGMapJob *job)
{
	_jobs.append(job);

	connect(job, &IMGMapJob::finished, this, &IMGMap::jobFinished);
	job->run();
}

void IMGMap::removeJob(IMGMapJob *job)
{
	_jobs.removeOne(job);
	job->deleteLater();
}

void IMGMap::jobFinished(IMGMapJob *job)
{
	const QList<IMG::RasterTile> &tiles = job->tiles();

	for (int i = 0; i < tiles.size(); i++) {
		const IMG::RasterTile &mt = tiles.at(i);
		if (mt.isValid())
			QPixmapCache::insert(mt.key(), mt.pixmap());
	}

	removeJob(job);

	emit tilesLoaded();
}

void IMGMap::cancelJobs()
{
	for (int i = 0; i < _jobs.size(); i++)
		_jobs.at(i)->cancel();
}

void IMGMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	Q_UNUSED(flags);

	QPointF tl(floor(rect.left() / TILE_SIZE)
	  * TILE_SIZE, floor(rect.top() / TILE_SIZE) * TILE_SIZE);
	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	int width = ceil(s.width() / TILE_SIZE);
	int height = ceil(s.height() / TILE_SIZE);

	QList<RasterTile> tiles;

	for (int n = 0; n < _data.size(); n++) {
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				QPixmap pm;
				QPoint ttl(tl.x() + i * TILE_SIZE, tl.y() + j * TILE_SIZE);
				QString key(_data.at(n)->fileName() + "-" + QString::number(_zoom)
				  + "_" + QString::number(ttl.x()) + "_" + QString::number(ttl.y()));

				if (isRunning(key))
					continue;

				if (QPixmapCache::find(key, &pm))
					painter->drawPixmap(ttl, pm);
				else {
					QList<MapData::Poly> polygons, lines;
					QList<MapData::Point> points;

					QRectF polyRect(ttl, QPointF(ttl.x() + TILE_SIZE,
					  ttl.y() + TILE_SIZE));
					polyRect &= _bounds;
					RectD polyRectD(_transform.img2proj(polyRect.topLeft()),
					  _transform.img2proj(polyRect.bottomRight()));
					_data.at(n)->polys(polyRectD.toRectC(_projection, 20), _zoom,
					  &polygons, &lines);

					QRectF pointRect(QPointF(ttl.x() - TEXT_EXTENT,
					  ttl.y() - TEXT_EXTENT), QPointF(ttl.x() + TILE_SIZE
					  + TEXT_EXTENT, ttl.y() + TILE_SIZE + TEXT_EXTENT));
					pointRect &= _bounds;
					RectD pointRectD(_transform.img2proj(pointRect.topLeft()),
					  _transform.img2proj(pointRect.bottomRight()));
					_data.at(n)->points(pointRectD.toRectC(_projection, 20),
					  _zoom, &points);

					tiles.append(RasterTile(_projection, _transform,
					  _data.at(n)->style(), _zoom,
					  QRect(ttl, QSize(TILE_SIZE, TILE_SIZE)), _tileRatio, key,
					  polygons, lines, points));
				}
			}
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
				QPixmapCache::insert(mt.key(), pm);
			}
		} else
			runJob(new IMGMapJob(tiles));
	}
}

void IMGMap::setDevicePixelRatio(qreal deviceRatio, qreal mapRatio)
{
	Q_UNUSED(mapRatio);

	_tileRatio = deviceRatio;
}

void IMGMap::setOutputProjection(const Projection &projection)
{
	if (projection == _projection)
		return;

	_projection = projection;
	// Limit the bounds for some well known projections
	// (world maps have N/S bounds up to 90/-90!)
	if (_projection == PCS::pcs(3857) || _projection == PCS::pcs(3395))
		_dataBounds = _data.first()->bounds() & OSM::BOUNDS;
	else if (_projection == PCS::pcs(3031) || _projection == PCS::pcs(3976))
		_dataBounds = _data.first()->bounds() & RectC(Coordinates(-180, -60),
		  Coordinates(180, -90));
	else if (_projection == PCS::pcs(3995) || _projection == PCS::pcs(3413))
		_dataBounds = _data.first()->bounds() & RectC(Coordinates(-180, 90),
		  Coordinates(180, 60));
	else
		_dataBounds = _data.first()->bounds();

	updateTransform();
	QPixmapCache::clear();
}

Map* IMGMap::create(const QString &path, const Projection &, bool *isDir)
{
	if (isDir)
		*isDir = GMAPData::isGMAP(path);

	return new IMGMap(path);
}
