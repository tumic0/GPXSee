#include <cmath>
#include <QString>
#include "wgs84.h"
#include "ellipsoid.h"


#define INTERNATIONAL_RADIUS     6378388.0
#define INTERNATIONAL_FLATTENING (1.0/297.0)
#define KRASSOVSKY_RADIUS        6378245.0
#define KRASSOVSKY_FLATTENING    (1.0/298.3)
#define CLARKE1866_RADIUS        6378206.4
#define CLARKE1866_FLATTENING    (1.0/294.9786982)
#define GRS80_RADIUS             6378137.0
#define GRS80_FLATTENING         (1.0/298.257222101)
#define WGS72_RADIUS             6378135.0
#define WGS72_FLATTENING         (1.0/298.26)
#define GRS67_RADIUS             6378160.0
#define GRS67_FLATTENING         (1.0/298.25)


// Must be in Ellipsoid::Name order!
struct {double radius; double flattening;} static ellipsoids[] = {
	{CLARKE1866_RADIUS, CLARKE1866_FLATTENING},
	{GRS80_RADIUS, GRS80_FLATTENING},
	{INTERNATIONAL_RADIUS, INTERNATIONAL_FLATTENING},
	{KRASSOVSKY_RADIUS, KRASSOVSKY_FLATTENING},
	{WGS84_RADIUS, WGS84_FLATTENING},
	{WGS72_RADIUS, WGS72_FLATTENING},
	{GRS67_RADIUS, GRS67_FLATTENING}
};

Ellipsoid::Ellipsoid()
{
	_radius = WGS84_RADIUS;
	_flattening = WGS84_FLATTENING;
}

Ellipsoid::Ellipsoid(Name name)
{
	_radius = ellipsoids[name].radius;
	_flattening = ellipsoids[name].flattening;
}
