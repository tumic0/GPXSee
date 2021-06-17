#ifndef POLYCONIC_H
#define POLYCONIC_H

#include "ct.h"

class Ellipsoid;

class Polyconic : public CT
{
public:
	Polyconic(const Ellipsoid &ellipsoid, double latitudeOrigin,
	  double longitudeOrigin, double falseEasting, double falseNorthing);

	virtual CT *clone() const {return new Polyconic(*this);}
	virtual bool operator==(const CT &ct) const;

	virtual PointD ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const PointD &p) const;

private:
	double _a;
	double _b;
	double _es2;
	double _es4;
	double _es6;
	double _M0;
	double _c0;
	double _c1;
	double _c2;
	double _c3;
	double _longitudeOrigin;
	double _latitudeOrigin;
	double _falseEasting;
	double _falseNorthing;
};

#endif // POLYCONIC_H
