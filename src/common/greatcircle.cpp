#include "greatcircle.h"

Coordinates GreatCircle::pointAt(const Coordinates &c1, const Coordinates &c2,
  double f)
{
	double lat1 = deg2rad(c1.lat());
	double lon1 = deg2rad(c1.lon());
	double lat2 = deg2rad(c2.lat());
	double lon2 = deg2rad(c2.lon());

	double cosLat1 = cos(lat1);
	double cosLat2 = cos(lat2);
	double d = 2 * asin(sqrt(pow((sin((lat1 - lat2) / 2)), 2) + cosLat1
	  * cosLat2 * pow(sin((lon1 - lon2) / 2), 2)));
	double sinD = sin(d);
	double A = sin((1.0-f)*d) / sinD;
	double B = sin(f*d) / sinD;
	double x = A * cosLat1 * cos(lon1) + B * cosLat2 * cos(lon2);
	double y = A * cosLat1 * sin(lon1) + B * cosLat2 * sin(lon2);
	double z = A * sin(lat1) + B * sin(lat2);

	return Coordinates(rad2deg(atan2(y, x)), rad2deg(atan2(z, sqrt(x*x + y*y))));
}
