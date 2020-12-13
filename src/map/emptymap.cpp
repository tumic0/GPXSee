#include <QtGlobal>
#include <QPainter>
#include "common/rectc.h"
#include "emptymap.h"


#define TILE_SIZE 256

static int limitZoom(int zoom)
{
	if (zoom < OSM::ZOOMS.min())
		return OSM::ZOOMS.min();
	if (zoom > OSM::ZOOMS.max())
		return OSM::ZOOMS.max();

	return zoom;
}


EmptyMap::EmptyMap(QObject *parent) : Map(QString(), parent)
{
	_zoom = OSM::ZOOMS.max();
}

QRectF EmptyMap::bounds()
{
	return QRectF(ll2xy(OSM::BOUNDS.topLeft()), ll2xy(OSM::BOUNDS.bottomRight()));
}

int EmptyMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (!rect.isValid())
		_zoom = OSM::ZOOMS.max();
	else {
		QRectF tbr(OSM::ll2m(rect.topLeft()), OSM::ll2m(rect.bottomRight()));
		QPointF sc(tbr.width() / size.width(), tbr.height() / size.height());

		_zoom = limitZoom(OSM::scale2zoom(qMax(sc.x(), -sc.y()), TILE_SIZE));
	}

	return _zoom;
}

qreal EmptyMap::resolution(const QRectF &rect)
{
	return OSM::resolution(rect.center(), _zoom, TILE_SIZE);
}

int EmptyMap::zoomIn()
{
	_zoom = qMin(_zoom + 1, OSM::ZOOMS.max());
	return _zoom;
}

int EmptyMap::zoomOut()
{
	_zoom = qMax(_zoom - 1, OSM::ZOOMS.min());
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
	qreal scale = OSM::zoom2scale(_zoom, TILE_SIZE);
	QPointF m = OSM::ll2m(c);
	return QPointF(m.x() / scale, m.y() / -scale);
}

Coordinates EmptyMap::xy2ll(const QPointF &p)
{
	qreal scale = OSM::zoom2scale(_zoom, TILE_SIZE);
	return OSM::m2ll(QPointF(p.x() * scale, -p.y() * scale));
}
