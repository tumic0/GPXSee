#include <cmath>
#include "common/coordinates.h"
#include "common/wgs84.h"
#include "webmercator.h"

PointD WebMercator::ll2xy(const Coordinates &c) const
{
	return PointD(deg2rad(c.lon()) * WGS84_RADIUS,
	  log(tan(M_PI_4 + deg2rad(c.lat())/2.0)) * WGS84_RADIUS);
}

Coordinates WebMercator::xy2ll(const PointD &p) const
{
	return Coordinates(rad2deg(p.x() / WGS84_RADIUS),
	  rad2deg(2.0 * atan(exp(p.y() / WGS84_RADIUS)) - M_PI_2));
}
