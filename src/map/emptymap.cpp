#include <QtGlobal>
#include <QPainter>
#include "common/coordinates.h"
#include "common/rectc.h"
#include "common/wgs84.h"
#include "emptymap.h"


#define ZOOM_MIN 0
#define ZOOM_MAX 19
#define TILE_SIZE 256

static QPointF ll2m(const Coordinates &c)
{
	return QPointF(c.lon(), rad2deg(log(tan(M_PI_4 + deg2rad(c.lat())/2.0))));
}

static Coordinates m2ll(const QPointF &p)
{
	return Coordinates(p.x(), rad2deg(2.0 * atan(exp(deg2rad(p.y()))) - M_PI_2));
}

static qreal zoom2scale(int zoom)
{
	return (360.0/(qreal)((1<<zoom) * TILE_SIZE));
}

static int scale2zoom(qreal scale)
{
	return (int)log2(360.0/(scale * (qreal)TILE_SIZE));
}

static int limitZoom(int zoom)
{
	if (zoom < ZOOM_MIN)
		return ZOOM_MIN;
	if (zoom > ZOOM_MAX)
		return ZOOM_MAX;

	return zoom;
}


EmptyMap::EmptyMap(QObject *parent) : Map(parent)
{
	_zoom = ZOOM_MAX;
}

QRectF EmptyMap::bounds()
{
	return QRectF(ll2xy(Coordinates(-180, 85)), ll2xy(Coordinates(180, -85)));
}

int EmptyMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (!rect.isValid())
		_zoom = ZOOM_MAX;
	else {
		QRectF tbr(ll2m(rect.topLeft()), ll2m(rect.bottomRight()));
		QPointF sc(tbr.width() / size.width(), tbr.height() / size.height());

		_zoom = limitZoom(scale2zoom(qMax(sc.x(), -sc.y())));
	}

	return _zoom;
}

qreal EmptyMap::resolution(const QRectF &rect)
{
	qreal scale = zoom2scale(_zoom);

	return (WGS84_RADIUS * 2.0 * M_PI * scale / 360.0
	  * cos(2.0 * atan(exp(deg2rad(-rect.center().y() * scale))) - M_PI/2));
}

int EmptyMap::zoomIn()
{
	_zoom = qMin(_zoom + 1, ZOOM_MAX);
	return _zoom;
}

int EmptyMap::zoomOut()
{
	_zoom = qMax(_zoom - 1, ZOOM_MIN);
	return _zoom;
}

void EmptyMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	Q_UNUSED(painter);
	Q_UNUSED(rect);
	Q_UNUSED(flags);
}

QPointF EmptyMap::ll2xy(const Coordinates &c)
{
	qreal scale = zoom2scale(_zoom);
	QPointF m = ll2m(c);
	return QPointF(m.x() / scale, m.y() / -scale);
}

Coordinates EmptyMap::xy2ll(const QPointF &p)
{
	qreal scale = zoom2scale(_zoom);
	return m2ll(QPointF(p.x() * scale, -p.y() * scale));
}
