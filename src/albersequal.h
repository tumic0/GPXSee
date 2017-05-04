#ifndef ALBERSEQUAL_H
#define ALBERSEQUAL_H

#include "projection.h"

class Ellipsoid;

class AlbersEqual : public Projection
{
public:
	AlbersEqual(const Ellipsoid &ellipsoid, double standardParallel1,
	  double standardParallel2, double latitudeOrigin, double longitudeOrigin,
	  double falseEasting, double falseNorthing);

	virtual QPointF ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const QPointF &p) const;

private:
	Ellipsoid _e;

	double _latitudeOrigin;
	double _longitudeOrigin;
	double _falseEasting;
	double _falseNorthing;

	double _rho0;
	double _C;
	double _n;
	double _es;
	double _es2;
	double _a_over_n;
	double _one_minus_es2;
	double _two_es;
};

#endif // ALBERSEQUAL_H
