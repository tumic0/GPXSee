#include <cmath>
#include "rd.h"
#include "mercator.h"

QPointF Mercator::ll2xy(const Coordinates &c) const
{
	return QPointF(c.lon(), rad2deg(log(tan(M_PI/4.0 + deg2rad(c.lat())/2.0))));
}

Coordinates Mercator::xy2ll(const QPointF &p) const
{
	return Coordinates(p.x(), rad2deg(2 * atan(exp(deg2rad(p.y()))) - M_PI/2));
}
