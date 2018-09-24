#include <QtGlobal>
#include <QPainter>
#include "common/rectc.h"
#include "osm.h"
#include "emptymap.h"


#define TILE_SIZE 256

static int limitZoom(int zoom)
{
	if (zoom < osm::zooms.min())
		return osm::zooms.min();
	if (zoom > osm::zooms.max())
		return osm::zooms.max();

	return zoom;
}


EmptyMap::EmptyMap(QObject *parent) : Map(parent)
{
	_zoom = osm::zooms.max();
}

QRectF EmptyMap::bounds()
{
	return QRectF(ll2xy(osm::bounds.topLeft()), ll2xy(osm::bounds.bottomRight()));
}

int EmptyMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (!rect.isValid())
		_zoom = osm::zooms.max();
	else {
		QRectF tbr(osm::ll2m(rect.topLeft()), osm::ll2m(rect.bottomRight()));
		QPointF sc(tbr.width() / size.width(), tbr.height() / size.height());

		_zoom = limitZoom(osm::scale2zoom(qMax(sc.x(), -sc.y()), TILE_SIZE));
	}

	return _zoom;
}

qreal EmptyMap::resolution(const QRectF &rect)
{
	return osm::resolution(rect.center(), _zoom, TILE_SIZE);
}

int EmptyMap::zoomIn()
{
	_zoom = qMin(_zoom + 1, osm::zooms.max());
	return _zoom;
}

int EmptyMap::zoomOut()
{
	_zoom = qMax(_zoom - 1, osm::zooms.min());
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
	qreal scale = osm::zoom2scale(_zoom, TILE_SIZE);
	QPointF m = osm::ll2m(c);
	return QPointF(m.x() / scale, m.y() / -scale);
}

Coordinates EmptyMap::xy2ll(const QPointF &p)
{
	qreal scale = osm::zoom2scale(_zoom, TILE_SIZE);
	return osm::m2ll(QPointF(p.x() * scale, -p.y() * scale));
}
