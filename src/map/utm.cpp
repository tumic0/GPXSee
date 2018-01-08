#include "ellipsoid.h"
#include "utm.h"

Projection::Setup UTM::setup(int zone)
{
	return Projection::Setup(0, (qAbs(zone) - 1)*6 - 180 + 3, 0.9996, 500000,
	  zone < 0 ? 10000000 : 0, 0, 0);
}

int UTM::zone(const Coordinates &c)
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

	return zone;
}
