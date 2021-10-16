#include "greatcircle.h"

#define DELTA 1e-5

static bool antipodes(const Coordinates &c1, const Coordinates &c2)
{
	return ((qAbs(c1.lat() + c2.lat()) < DELTA)
	  && (qAbs(180.0 - qAbs(c1.lon() - c2.lon())) < DELTA));
}

GreatCircle::GreatCircle(const Coordinates &c1, const Coordinates &c2)
{
	double lat1, lon1, lat2, lon2;

	if (antipodes(c1, c2)) {
		/* In case of antipodes (which would lead to garbage output without
		   this hack), move the points DELTA degrees closer to each other in
		   a way that the route never crosses the antimeridian. */
		if (c1.lon() < c2.lon()) {
			lon1 = deg2rad(c1.lon() + DELTA);
			lon2 = deg2rad(c2.lon() - DELTA);
		} else {
			lon1 = deg2rad(c1.lon() - DELTA);
			lon2 = deg2rad(c2.lon() + DELTA);
		}
		if (c1.lat() < c2.lat()) {
			lat1 = deg2rad(c1.lat() + DELTA);
			lat2 = deg2rad(c2.lat() - DELTA);
		} else {
			lat1 = deg2rad(c1.lat() - DELTA);
			lat2 = deg2rad(c2.lat() + DELTA);
		}
	} else {
		lat1 = deg2rad(c1.lat());
		lon1 = deg2rad(c1.lon());
		lat2 = deg2rad(c2.lat());
		lon2 = deg2rad(c2.lon());
	}

	double cosLat1 = cos(lat1);
	double cosLat2 = cos(lat2);
	_sinLat1 = sin(lat1);
	_sinLat2 = sin(lat2);

	_xA = cosLat1 * cos(lon1);
	_xB = cosLat2 * cos(lon2);
	_yA = cosLat1 * sin(lon1);
	_yB = cosLat2 * sin(lon2);

	_d = 2 * asin(sqrt(pow((sin((lat1 - lat2) / 2)), 2) + cosLat1 * cosLat2
	  * pow(sin((lon1 - lon2) / 2), 2)));
	_sinD = sin(_d);
}

Coordinates GreatCircle::pointAt(double f) const
{
	double A = sin((1.0 - f) * _d) / _sinD;
	double B = sin(f * _d) / _sinD;
	double x = A * _xA + B * _xB;
	double y = A * _yA + B * _yB;
	double z = A * _sinLat1 + B * _sinLat2;

	return Coordinates(rad2deg(atan2(y, x)), rad2deg(atan2(z, sqrt(x*x + y*y))));
}
