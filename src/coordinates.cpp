#include "rd.h"
#include "wgs84.h"
#include "coordinates.h"

#define MIN_LAT deg2rad(-90.0)
#define MAX_LAT deg2rad(90.0)
#define MIN_LON deg2rad(-180.0)
#define MAX_LON deg2rad(180.0)

qreal Coordinates::distanceTo(const Coordinates &c) const
{
	qreal dLat = deg2rad(c.lat() - _lat);
	qreal dLon = deg2rad(c.lon() - _lon);
	qreal a = pow(sin(dLat / 2.0), 2.0)
	  + cos(deg2rad(_lat)) * cos(deg2rad(c.lat())) * pow(sin(dLon / 2.0), 2.0);

	return (WGS84_RADIUS * (2.0 * atan2(sqrt(a), sqrt(1.0 - a))));
}

QDebug operator<<(QDebug dbg, const Coordinates &coordinates)
{
	dbg.nospace() << "Coordinates(" << coordinates.lon() << ", "
	  << coordinates.lat() << ")";

	return dbg.space();
}

QPair<Coordinates, Coordinates> Coordinates::boundingRect(qreal distance) const
{
	qreal radDist = distance / WGS84_RADIUS;

	qreal minLat = deg2rad(_lat) - radDist;
	qreal maxLat = deg2rad(_lat) + radDist;

	qreal minLon, maxLon;
	if (minLat > MIN_LAT && maxLat < MAX_LAT) {
		qreal deltaLon = asin(sin(radDist) / cos(_lat));
		minLon = deg2rad(_lon) - deltaLon;
		if (minLon < MIN_LON)
			minLon += 2.0 * M_PI;
		maxLon = deg2rad(_lon) + deltaLon;
		if (maxLon > MAX_LON)
			maxLon -= 2.0 * M_PI;
	} else {
		// a pole is within the distance
		minLat = qMax(minLat, MIN_LAT);
		maxLat = qMin(maxLat, MAX_LAT);
		minLon = MIN_LON;
		maxLon = MAX_LON;
	}

	return QPair<Coordinates, Coordinates>(Coordinates(rad2deg(qMin(minLon,
	  maxLon)), rad2deg(qMin(minLat, maxLat))), Coordinates(rad2deg(qMax(minLon,
	  maxLon)), rad2deg(qMax(minLat, maxLat))));
}
