#include <cmath>
#include <QString>
#include "wgs84.h"
#include "ellipsoid.h"


#define INTERNATIONAL_RADIUS     6378388.0
#define INTERNATIONAL_FLATTENING (1.0/297.0)
#define KRASSOVSKY_RADIUS        6378245.0
#define KRASSOVSKY_FLATTENING    (1.0/298.3)
#define BESSEL_RADIUS            6377397.155
#define BESSEL_FLATTENING        (1.0/299.1528128)
#define GRS80_RADIUS             6378137.0
#define GRS80_FLATTENING         (1.0/298.257222101)
#define WGS70_RADIUS             6378135.0
#define WGS70_FLATTENING         (1.0/298.26)
#define SAD69_RADIUS             6378160.0
#define SAD69_FLATTENING         (1.0/298.25)


#ifndef M_PI_2
	#define M_PI_2 1.57079632679489661923
#endif // M_PI_2
#ifndef M_PI_4
	#define M_PI_4 0.78539816339744830962
#endif // M_PI_4

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

typedef struct {
	const char *name;
	double radius;
	double flattening;
} Entry;

static Entry list[] = {
	{"S42", KRASSOVSKY_RADIUS, KRASSOVSKY_FLATTENING},
	{"Pulkovo 1942", KRASSOVSKY_RADIUS, KRASSOVSKY_FLATTENING},
	{"European 1950", INTERNATIONAL_RADIUS, INTERNATIONAL_FLATTENING},
	{"European 1979", INTERNATIONAL_RADIUS, INTERNATIONAL_FLATTENING},
	{"NZGD1949", INTERNATIONAL_RADIUS, INTERNATIONAL_FLATTENING},
	{"NAD27", BESSEL_RADIUS, BESSEL_FLATTENING},
	{"NAD83", GRS80_RADIUS, GRS80_FLATTENING},
	{"WGS 72", WGS70_RADIUS, WGS70_FLATTENING},
	{"South American 1969", SAD69_RADIUS, SAD69_FLATTENING}
};


Ellipsoid::Ellipsoid()
{
	_radius = WGS84_RADIUS;
	_flattening = WGS84_FLATTENING;
}

Ellipsoid::Ellipsoid(const QString &datum)
{
	for (size_t i = 0; i < ARRAY_SIZE(list); i++) {
		if (datum.startsWith(list[i].name)) {
			_radius = list[i].radius;
			_flattening = list[i].flattening;
			return;
		}
	}

	_radius = WGS84_RADIUS;
	_flattening = WGS84_FLATTENING;
}

double Ellipsoid::q(double b) const
{
	double e = sqrt(_flattening * (2. - _flattening));
	double esb = e * sin(b);
	return log(tan(M_PI_4 + b / 2.) * pow((1. - esb) / (1. + esb), e / 2.));
}

double Ellipsoid::iq(double q) const
{
	double e = sqrt(_flattening * (2. - _flattening));
	double b0 = 0.;
	double b = 2. * atan(exp(q)) - M_PI_2;

	do {
		b0 = b;
		double esb = e * sin(b);
		b = 2. * atan(exp(q) * pow((1. - esb) / (1. + esb), -e / 2.)) - M_PI_2;
	} while (fabs(b - b0) > 1e-10);

	return b;
}

double Ellipsoid::nradius(double phi) const
{
	double sin_phi = sin(phi);
	return (_radius / sqrt(1. - (_flattening * (2. - _flattening)) * sin_phi
	  * sin_phi));
}
