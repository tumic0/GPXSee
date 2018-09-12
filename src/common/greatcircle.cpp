#include "greatcircle.h"

GreatCircle::GreatCircle(const Coordinates &c1, const Coordinates &c2)
{
	double lat1 = deg2rad(c1.lat());
	double lon1 = deg2rad(c1.lon());
	double lat2 = deg2rad(c2.lat());
	double lon2 = deg2rad(c2.lon());

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
