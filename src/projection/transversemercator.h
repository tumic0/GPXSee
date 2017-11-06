#ifndef TRANSVERSEMERCATOR_H
#define TRANSVERSEMERCATOR_H

#include "projection.h"
#include "ellipsoid.h"

class TransverseMercator : public Projection
{
public:
	TransverseMercator(const Ellipsoid &ellipsoid, double latitudeOrigin,
	  double longitudeOrigin, double scale, double falseEasting,
	  double falseNorthing);

	virtual QPointF ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const QPointF &p) const;

private:
	Ellipsoid _e;
	double _longitudeOrigin;
	double _latitudeOrigin;
	double _scale;
	double _falseEasting;
	double _falseNorthing;

	double _es;
	double _ebs;
	double _ap, _bp, _cp, _dp, _ep;
};

#endif // TRANSVERSEMERCATOR_H
