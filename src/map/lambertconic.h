#ifndef LAMBERTCONIC_H
#define LAMBERTCONIC_H

#include "ct.h"

class Ellipsoid;

class LambertConic1 : public CT
{
public:
	LambertConic1() {}
	LambertConic1(const Ellipsoid *ellipsoid, double latitudeOrigin,
	  double longitudeOrigin, double scale, double falseEasting,
	  double falseNorthing);

	virtual CT *clone() const {return new LambertConic1(*this);}

	virtual PointD ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const PointD &p) const;

private:
	double _longitudeOrigin;
	double _falseEasting;
	double _falseNorthing;

	double _e;
	double _e_over_2;
	double _n;
	double _t0;
	double _rho0;
	double _rho_olat;
};

class LambertConic2 : public CT
{
public:
	LambertConic2(const Ellipsoid *ellipsoid, double standardParallel1,
	  double standardParallel2, double latitudeOrigin, double longitudeOrigin,
	   double falseEasting, double falseNorthing);

	virtual CT *clone() const {return new LambertConic2(*this);}

	virtual PointD ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const PointD &p) const;

private:
	LambertConic1 _lc1;
};

#endif // LAMBERTCONIC_H
