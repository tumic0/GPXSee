#ifndef TRANSVERSEMERCATOR_H
#define TRANSVERSEMERCATOR_H

#include "projection.h"

class Ellipsoid;

class TransverseMercator : public Projection
{
public:
	TransverseMercator(const Ellipsoid &ellipsoid, double centralMeridian,
	  double scale, double falseEasting, double falseNorthing);

	virtual QPointF ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const QPointF &p) const;

private:
	double _centralMeridian;
	double _scale;
	double _falseEasting;
	double _falseNorthing;

	double _rectifyingRadius;
	double _A, _B, _C, _D;
	double _beta1, _beta2, _beta3, _beta4;
	double _delta1, _delta2, _delta3, _delta4;
	double _AStar, _BStar, _CStar, _DStar;
};

#endif // TRANSVERSEMERCATOR_H
