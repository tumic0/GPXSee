#include "wgs84.h"
#include "coordinates.h"

double Coordinates::distanceTo(const Coordinates &c) const
{
	double dLat = deg2rad(c.lat() - _lat);
	double dLon = deg2rad(c.lon() - _lon);
	double a = pow(sin(dLat / 2.0), 2.0)
	  + cos(deg2rad(_lat)) * cos(deg2rad(c.lat())) * pow(sin(dLon / 2.0), 2.0);

	return (WGS84_RADIUS * (2.0 * atan2(sqrt(a), sqrt(1.0 - a))));
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Coordinates &c)
{
	dbg.nospace() << "Coordinates(" << c.lon() << ", " << c.lat() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
