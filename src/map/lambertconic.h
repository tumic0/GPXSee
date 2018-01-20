#ifndef LAMBERTCONIC_H
#define LAMBERTCONIC_H

#include "ellipsoid.h"
#include "projection.h"

class LambertConic1 : public Projection
{
public:
	LambertConic1() {}
	LambertConic1(const Ellipsoid *ellipsoid, double latitudeOrigin,
	  double longitudeOrigin, double scale, double falseEasting,
	  double falseNorthing);

	virtual QPointF ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const QPointF &p) const;

private:
	double _longitudeOrigin;
	double _falseEasting;
	double _falseNorthing;

	double _es;
	double _es_over_2;
	double _n;
	double _t0;
	double _rho0;
	double _rho_olat;
};

class LambertConic2 : public Projection
{
public:
	LambertConic2(const Ellipsoid *ellipsoid, double standardParallel1,
	  double standardParallel2, double latitudeOrigin, double longitudeOrigin,
	   double falseEasting, double falseNorthing);

	virtual QPointF ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const QPointF &p) const;

private:
	LambertConic1 _lc1;
};

#endif // LAMBERTCONIC_H
