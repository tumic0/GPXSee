#include "greatcircle.h"

GreatCircle::GreatCircle(const Coordinates &c1, const Coordinates &c2)
{
	_lat1 = deg2rad(c1.lat());
	_lon1 = deg2rad(c1.lon());
	_lat2 = deg2rad(c2.lat());
	_lon2 = deg2rad(c2.lon());

	_cosLat1 = cos(_lat1);
	_cosLat2 = cos(_lat2);
	_d = 2 * asin(sqrt(pow((sin((_lat1 - _lat2) / 2)), 2) + _cosLat1
	  * _cosLat2 * pow(sin((_lon1 - _lon2) / 2), 2)));
	_sinD = sin(_d);
}

Coordinates GreatCircle::pointAt(double f) const
{
	double A = sin((1.0 - f) * _d) / _sinD;
	double B = sin(f*_d) / _sinD;
	double x = A * _cosLat1 * cos(_lon1) + B * _cosLat2 * cos(_lon2);
	double y = A * _cosLat1 * sin(_lon1) + B * _cosLat2 * sin(_lon2);
	double z = A * sin(_lat1) + B * sin(_lat2);

	return Coordinates(rad2deg(atan2(y, x)), rad2deg(atan2(z, sqrt(x*x + y*y))));
}
