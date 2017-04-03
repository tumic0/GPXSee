#ifndef LAMBERTCONIC_H
#define LAMBERTCONIC_H

#include "ellipsoid.h"
#include "projection.h"

class LambertConic : public Projection
{
public:
	LambertConic(const Ellipsoid &ellipsoid, double standardParallel1,
	  double standardParallel2, double centralParallel, double centralMeridian,
	  double scale, double falseEasting, double falseNorthing);

	virtual QPointF ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const QPointF &p) const;

private:
	Ellipsoid _e;

	double _cm;
	double _fe;
	double _fn;

	double _q0;
	double _R0;
	double _n;
};

#endif // LAMBERTCONIC_H
