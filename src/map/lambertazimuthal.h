#ifndef LAMBERTAZIMUTHAL_H
#define LAMBERTAZIMUTHAL_H

#include "ellipsoid.h"
#include "projection.h"

class LambertAzimuthal : public Projection
{
public:
	LambertAzimuthal(const Ellipsoid &ellipsoid, double latitudeOrigin,
	  double longitudeOrigin, double falseEasting, double falseNorthing);

	virtual QPointF ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const QPointF &p) const;

private:
	double _ra;
	double _sinLatOrigin;
	double _cosLatOrigin;
	double _absLatOrigin;

	double _latOrigin;
	double _lonOrigin;
	double _falseNorthing;
	double _falseEasting;
};

#endif // LAMBERTAZIMUTHAL_H
