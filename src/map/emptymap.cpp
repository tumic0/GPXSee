#include <QtGlobal>
#include <QPainter>
#include "rectc.h"
#include "misc.h"
#include "rd.h"
#include "wgs84.h"
#include "coordinates.h"
#include "mercator.h"
#include "emptymap.h"


#define SCALE_MIN 0.5
#define SCALE_MAX 1.0E-6

EmptyMap::EmptyMap(QObject *parent) : Map(parent)
{
	_scale = SCALE_MAX;
}

QRectF EmptyMap::bounds() const
{
	return scaled(QRectF(QPointF(-180, -180), QSizeF(360, 360)), 1.0/_scale);
}

qreal EmptyMap::zoomFit(const QSize &size, const RectC &br)
{
	if (!br.isValid())
		_scale = SCALE_MAX;
	else {
		QRectF tbr(Mercator().ll2xy(br.topLeft()),
		  Mercator().ll2xy(br.bottomRight()));
		QPointF sc(tbr.width() / size.width(), tbr.height() / size.height());
		_scale = qMax(sc.x(), sc.y());
	}

	_scale = qMax(_scale, SCALE_MAX);
	_scale = qMin(_scale, SCALE_MIN);

	return _scale;
}

qreal EmptyMap::zoomFit(qreal resolution, const Coordinates &c)
{
	_scale = (360.0 * resolution) / (WGS84_RADIUS * 2 * M_PI
	  * cos(deg2rad(c.lat())));

	_scale = qMax(_scale, SCALE_MAX);
	_scale = qMin(_scale, SCALE_MIN);

	return _scale;
}

qreal EmptyMap::resolution(const QPointF &p) const
{
	return (WGS84_RADIUS * 2 * M_PI * _scale / 360.0
	  * cos(2.0 * atan(exp(deg2rad(-p.y() * _scale))) - M_PI/2));
}

qreal EmptyMap::zoomIn()
{
	_scale = qMax(_scale / 2.0, SCALE_MAX);
	return _scale;
}

qreal EmptyMap::zoomOut()
{
	_scale = qMin(_scale * 2.0, SCALE_MIN);
	return _scale;
}

void EmptyMap::draw(QPainter *painter, const QRectF &rect)
{
	painter->fillRect(rect, _backgroundColor);
}

QPointF EmptyMap::ll2xy(const Coordinates &c)
{
	QPointF m = Mercator().ll2xy(c);
	return QPointF(m.x() / _scale, m.y() / -_scale);
}

Coordinates EmptyMap::xy2ll(const QPointF &p)
{
	QPointF m(p.x() * _scale, -p.y() * _scale);
	return Mercator().xy2ll(m);
}
