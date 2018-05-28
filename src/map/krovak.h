#ifndef KROVAK_H
#define KROVAK_H

#include "ct.h"

class Ellipsoid;

class Krovak : public CT
{
public:
	enum Orientation {
		North,
		South
	};

	Krovak(const Ellipsoid *ellipsoid, double standardParallel,
	  double azimuth, double scale, double centerLatitude,
	  double longitudeOrigin, double falseEasting, double falseNorthing,
	  Orientation orientation);

	virtual CT *clone() const {return new Krovak(*this);}

	virtual PointD ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const PointD &p) const;

private:
	double _e, _A, _B, _t0, _n, _r0, _phiP;
	double _cosAlphaC, _sinAlphaC, _lambda0, _FE, _FN;
	Orientation _orientation;
};

#endif // KROVAK_H
