#ifndef ALBERSEQUAL_H
#define ALBERSEQUAL_H

#include "ct.h"

class Ellipsoid;

class AlbersEqual : public CT
{
public:
	AlbersEqual(const Ellipsoid *ellipsoid, double standardParallel1,
	  double standardParallel2, double latitudeOrigin, double longitudeOrigin,
	  double falseEasting, double falseNorthing);

	virtual CT *clone() const {return new AlbersEqual(*this);}

	virtual PointD ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const PointD &p) const;

private:
	double _latitudeOrigin;
	double _longitudeOrigin;
	double _falseEasting;
	double _falseNorthing;

	double _a2;
	double _rho0;
	double _C;
	double _n;
	double _e;
	double _es;
	double _a_over_n;
	double _one_minus_es;
	double _two_e;
};

#endif // ALBERSEQUAL_H
