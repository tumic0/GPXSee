#ifndef GREATCIRCLE_H
#define GREATCIRCLE_H

#include "coordinates.h"

class GreatCircle
{
public:
	GreatCircle(const Coordinates &c1, const Coordinates &c2);

	Coordinates pointAt(double f) const;

private:
	double _lat1, _lon1, _lat2, _lon2;
	double _cosLat1, _cosLat2;
	double _d;
	double _sinD;
};

#endif // GREATCIRCLE_H
