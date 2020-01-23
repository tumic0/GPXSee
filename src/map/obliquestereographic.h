#ifndef OBLIQUESTEREOGRAPHIC_H
#define OBLIQUESTEREOGRAPHIC_H

#include "ct.h"

class Ellipsoid;

class ObliqueStereographic : public CT
{
public:
	ObliqueStereographic(const Ellipsoid *ellipsoid, double latitudeOrigin,
	  double longitudeOrigin, double scale, double falseEasting,
	  double falseNorthing);
	virtual CT *clone() const {return new ObliqueStereographic(*this);}

	virtual PointD ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const PointD &p) const;

private:
	double _e, _es;
	double _chi0, _sinChi0, _cosChi0;
	double _lambda0;
	double _n;
	double _c;
	double _fe, _fn;
	double _twoRk0,	_g, _h;
};

#endif // OBLIQUESTEREOGRAPHIC_H
