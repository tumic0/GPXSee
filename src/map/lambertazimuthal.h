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
	virtual bool operator==(const CT &ct) const;

	virtual PointD ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const PointD &p) const;

private:
	double _lon0;
	double _fn, _fe;
	double _a, _e, _es, _qP, _beta0, _rq, _d;
};

#endif // LAMBERTAZIMUTHAL_H
