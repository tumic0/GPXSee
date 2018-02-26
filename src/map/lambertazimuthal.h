#ifndef LAMBERTAZIMUTHAL_H
#define LAMBERTAZIMUTHAL_H

#include "ct.h"

class Ellipsoid;

class LambertAzimuthal : public CT
{
public:
	LambertAzimuthal(const Ellipsoid *ellipsoid, double latitudeOrigin,
	  double longitudeOrigin, double falseEasting, double falseNorthing);

	virtual CT *clone() const {return new LambertAzimuthal(*this);}

	virtual QPointF ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const QPointF &p) const;

private:
	double _lon0;
	double _falseNorthing;
	double _falseEasting;
	double _a, _es, _es2, _qP, _beta0, _Rq, _D;
};

#endif // LAMBERTAZIMUTHAL_H
