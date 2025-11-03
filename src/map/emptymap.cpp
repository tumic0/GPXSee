#include <QtGlobal>
#include "common/rectc.h"
#include "emptymap.h"

#define TILE_SIZE 256

using namespace OSM;

static int limitZoom(int zoom)
{
	if (zoom < ZOOMS.min())
		return ZOOMS.min();
	if (zoom > ZOOMS.max())
		return ZOOMS.max();

	return zoom;
}

EmptyMap::EmptyMap(QObject *parent) : Map(QString(), parent)
{
	_zoom = ZOOMS.max();
	_scale = zoom2scale(_zoom, TILE_SIZE);
}

QRectF EmptyMap::bounds()
{
	return QRectF(ll2xy(BOUNDS.topLeft()), ll2xy(BOUNDS.bottomRight()));
}

qreal EmptyMap::resolution(const QRectF &rect)
{
	return OSM::resolution(rect.center(), _zoom, TILE_SIZE);
}

int EmptyMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (!rect.isValid())
		_zoom = ZOOMS.max();
	else {
		QRectF tbr(ll2m(rect.topLeft()), ll2m(rect.bottomRight()));
		QPointF sc(tbr.width() / size.width(), tbr.height() / size.height());

		_zoom = limitZoom(scale2zoom(qMax(sc.x(), -sc.y()), TILE_SIZE));
	}

	_scale = zoom2scale(_zoom, TILE_SIZE);

	return _zoom;
}

void EmptyMap::setZoom(int zoom)
{
	_zoom = zoom;
	_scale = zoom2scale(_zoom, TILE_SIZE);
}

int EmptyMap::zoomIn()
{
	_zoom = qMin(_zoom + 1, ZOOMS.max());
	_scale = zoom2scale(_zoom, TILE_SIZE);

	return _zoom;
}

int EmptyMap::zoomOut()
{
	_zoom = qMax(_zoom - 1, ZOOMS.min());
	_scale = zoom2scale(_zoom, TILE_SIZE);

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
	QPointF m = ll2m(c);
	return QPointF(m.x(), -m.y()) / _scale;
}

Coordinates EmptyMap::xy2ll(const QPointF &p)
{
	return m2ll(QPointF(p.x(), -p.y()) * _scale);
}
