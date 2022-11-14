#ifndef LINEC_H
#define LINEC_H

#include "coordinates.h"

class LineC
{
public:
	LineC(const Coordinates &c1, const Coordinates &c2) : _c1(c1), _c2(c2) {}

	const Coordinates &c1() const {return _c1;}
	const Coordinates &c2() const {return _c2;}

	Coordinates pointAt(double t) const
	{
		return Coordinates(
		  _c1.lon() + (_c2.lon() - _c1.lon()) * t,
		  _c1.lat() + (_c2.lat() - _c1.lat()) * t);
	}

private:
	Coordinates _c1, _c2;
};

#endif // LINEC_H
