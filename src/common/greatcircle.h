#ifndef GREATCIRCLE_H
#define GREATCIRCLE_H

#include "coordinates.h"

class GreatCircle
{
public:
	GreatCircle(const Coordinates &c1, const Coordinates &c2);

	Coordinates pointAt(double f) const;

private:
	double _xA, _xB, _yA, _yB;
	double _d;
	double _sinD;
	double _sinLat1, _sinLat2;
};

#endif // GREATCIRCLE_H
