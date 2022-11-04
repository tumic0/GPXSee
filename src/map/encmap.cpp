#include <QPainter>
#include <QPixmapCache>
#include <QtConcurrent>
#include "common/range.h"
#include "common/wgs84.h"
#include "ENC/rastertile.h"
#include "rectd.h"
#include "pcs.h"
#include "encmap.h"

#define TILE_SIZE 512
#define TEXT_EXTENT 160

using namespace ENC;

static Range ZOOMS = Range(0, 20);

ENCMap::ENCMap(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _data(fileName), _projection(PCS::pcs(3857)),
  _tileRatio(1.0), _zoom(0)
{
	if (_data.isValid()) {
		_llBounds = _data.bounds();
		updateTransform();
	}
}

void ENCMap::load()
{
	_data.load();
}

void ENCMap::unload()
{
	_data.clear();
}

int ENCMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (rect.isValid()) {
		RectD pr(rect, _projection, 10);

		_zoom = ZOOMS.min();
		for (int i = ZOOMS.min() + 1; i <= ZOOMS.max(); i++) {
			Transform t(transform(i));
			QRectF r(t.proj2img(pr.topLeft()), t.proj2img(pr.bottomRight()));
			if (size.width() < r.width() || size.height() < r.height())
				break;
			_zoom = i;
		}
	} else
		_zoom = ZOOMS.max();

	updateTransform();

	return _zoom;
}

int ENCMap::zoomIn()
{
	_zoom = qMin(_zoom + 1, ZOOMS.max());
	updateTransform();
	return _zoom;
}

int ENCMap::zoomOut()
{
	_zoom = qMax(_zoom - 1, ZOOMS.min());
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

			QPixmap pm;
			if (QPixmapCache::find(key(_zoom, ttl), &pm))
				painter->drawPixmap(ttl, pm);
			else {
				QList<MapData::Poly*> polygons;
				QList<MapData::Line*> lines;
				QList<MapData::Point*> points;

				QRectF polyRect(ttl, QPointF(ttl.x() + TILE_SIZE,
				  ttl.y() + TILE_SIZE));
				polyRect &= _bounds;
				RectD polyRectD(_transform.img2proj(polyRect.topLeft()),
				  _transform.img2proj(polyRect.bottomRight()));
				RectC polyRectC(polyRectD.toRectC(_projection, 20));
				_data.lines(polyRectC, &lines);
				_data.polygons(polyRectC, &polygons);

				QRectF pointRect(QPointF(ttl.x() - TEXT_EXTENT,
				  ttl.y() - TEXT_EXTENT), QPointF(ttl.x() + TILE_SIZE
				  + TEXT_EXTENT, ttl.y() + TILE_SIZE + TEXT_EXTENT));
				pointRect &= _bounds;
				RectD pointRectD(_transform.img2proj(pointRect.topLeft()),
				  _transform.img2proj(pointRect.bottomRight()));
				_data.points(pointRectD.toRectC(_projection, 20), &points);

				tiles.append(RasterTile(_projection, _transform, _zoom,
				  QRect(ttl, QSize(TILE_SIZE, TILE_SIZE)), _tileRatio,
				  lines, polygons, points));
			}
		}
	}

	QFuture<void> future = QtConcurrent::map(tiles, &RasterTile::render);
	future.waitForFinished();

	for (int i = 0; i < tiles.size(); i++) {
		const RasterTile &mt = tiles.at(i);
		const QPixmap &pm = mt.pixmap();
		painter->drawPixmap(mt.xy(), pm);
		QPixmapCache::insert(key(mt.zoom(), mt.xy()), pm);
	}
}

void ENCMap::setDevicePixelRatio(qreal deviceRatio, qreal mapRatio)
{
	Q_UNUSED(mapRatio);

	_tileRatio = deviceRatio;
}

void ENCMap::setOutputProjection(const Projection &projection)
{
	if (projection == _projection)
		return;

	_projection = projection;
	updateTransform();
	QPixmapCache::clear();
}

Map *ENCMap::create(const QString &path, const Projection &, bool *isMap)
{
	if (isMap)
		*isMap = false;

	return new ENCMap(path);
}
