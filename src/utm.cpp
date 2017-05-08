#include "ellipsoid.h"
#include "utm.h"

UTM::UTM(const Ellipsoid &ellipsoid, int zone)
  : _tm(ellipsoid, 0, (qAbs(zone) - 1)*6 - 180 + 3, 0.9996, 500000,
  zone < 0 ? 10000000 : 0)
{
}

UTM::UTM(const Ellipsoid &ellipsoid, const Coordinates &c)
  : _tm(ellipsoid, 0, 0, 0, 0, 0)
{
	int zone = int((c.lon() + 180)/6) + 1;

	if (c.lat() >= 56.0 && c.lat() < 64.0 && c.lon() >= 3.0 && c.lon() < 12.0)
		zone = 32;
	if (c.lat() >= 72.0 && c.lat() < 84.0) {
		if (c.lon() >= 0.0  && c.lon() <  9.0)
			zone = 31;
		else if (c.lon() >= 9.0  && c.lon() < 21.0)
			zone = 33;
		else if (c.lon() >= 21.0 && c.lon() < 33.0)
			zone = 35;
		else if (c.lon() >= 33.0 && c.lon() < 42.0)
			zone = 37;
	}
	double cm = (zone - 1)*6 - 180 + 3;

	_tm = TransverseMercator(ellipsoid, 0, cm, 0.9996, 500000,
	  (c.lat() < 0) ? 10000000 : 0);
}
