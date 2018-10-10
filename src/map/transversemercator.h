#ifndef TRANSVERSEMERCATOR_H
#define TRANSVERSEMERCATOR_H

#include "ct.h"

class Ellipsoid;

class TransverseMercator : public CT
{
public:
	TransverseMercator(const Ellipsoid *ellipsoid, double latitudeOrigin,
	  double longitudeOrigin, double scale, double falseEasting,
	  double falseNorthing);

	virtual CT *clone() const {return new TransverseMercator(*this);}

	virtual PointD ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const PointD &p) const;

private:
	double _longitudeOrigin;
	double _latitudeOrigin;
	double _scale;
	double _falseEasting;
	double _falseNorthing;
	double _a;
	double _es;
	double _ebs;
	double _ap, _bp, _cp, _dp, _ep;
};

#endif // TRANSVERSEMERCATOR_H
