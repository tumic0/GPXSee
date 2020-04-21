#ifndef POLARSTEREOGRAPHIC_H
#define POLARSTEREOGRAPHIC_H

#include "ct.h"

class Ellipsoid;

class PolarStereographic  : public CT
{
public:
	PolarStereographic(const Ellipsoid *ellipsoid, double latitudeOrigin,
	  double longitudeOrigin, double falseEasting, double falseNorthing);

	virtual CT *clone() const {return new PolarStereographic(*this);}
	virtual bool operator==(const CT &ct) const;

	virtual PointD ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const PointD &p) const;

private:
	double _originLatitude;
	double _originLongitude;
	double _falseEasting;
	double _falseNorthing;

	double _a_mc;
	double _es;
	double _es_OVER_2;
	double _two_a;
	double _mc;
	double _tc;
	double _e4;
	int _southernHemisphere;
};

#endif // POLARSTEREOGRAPHIC_H
