#include <cmath>
#include "common/coordinates.h"
#include "common/wgs84.h"
#include "mercator.h"

PointD Mercator::ll2xy(const Coordinates &c) const
{
	return PointD(deg2rad(c.lon()) * WGS84_RADIUS,
	  log(tan(M_PI/4.0 + deg2rad(c.lat())/2.0)) * WGS84_RADIUS);
}

Coordinates Mercator::xy2ll(const PointD &p) const
{
	return Coordinates(rad2deg(p.x() / WGS84_RADIUS),
	  rad2deg(2 * atan(exp(p.y() / WGS84_RADIUS)) - M_PI/2));
}
