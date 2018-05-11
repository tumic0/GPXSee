#ifndef GEOCENTRIC_H
#define GEOCENTRIC_H

#include <cmath>
#include "common/coordinates.h"

class Datum;

class Point3D {
public:
	Point3D() : _x(NAN), _y(NAN), _z(NAN) {}
	Point3D(double x, double y, double z) : _x(x), _y(y), _z(z) {}

	double x() const {return _x;}
	double y() const {return _y;}
	double z() const {return _z;}

private:
	double _x;
	double _y;
	double _z;
};

namespace Geocentric {
	Point3D fromGeodetic(const Coordinates &c, const Datum &datum);
	Coordinates toGeodetic(const Point3D &p, const Datum &datum);
}

#endif // GEOCENTRIC_H
