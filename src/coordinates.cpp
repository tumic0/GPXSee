#include "rd.h"
#include "wgs84.h"
#include "coordinates.h"


qreal Coordinates::distanceTo(const Coordinates &c) const
{
	qreal dLat = deg2rad(c.lat() - _lat);
	qreal dLon = deg2rad(c.lon() - _lon);
	qreal a = pow(sin(dLat / 2.0), 2.0)
	  + cos(deg2rad(_lat)) * cos(deg2rad(c.lat())) * pow(sin(dLon / 2.0), 2.0);

	return (WGS84_RADIUS * (2.0 * atan2(sqrt(a), sqrt(1.0 - a))));
}

QPointF Coordinates::toMercator() const
{
	return QPointF(_lon, rad2deg(log(tan(M_PI/4.0 + deg2rad(_lat)/2.0))));
}

Coordinates Coordinates::fromMercator(const QPointF &m)
{
	return Coordinates(m.x(), rad2deg(2 * atan(exp(deg2rad(m.y()))) - M_PI/2));
}

bool operator==(const Coordinates &c1, const Coordinates &c2)
{
	return (c1.lat() == c2.lat() && c1.lon() == c2.lon());
}

QDebug operator<<(QDebug dbg, const Coordinates &coordinates)
{
	dbg.nospace() << "Coordinates(" << coordinates.lon() << ", "
	  << coordinates.lat() << ")";

	return dbg.maybeSpace();
}
