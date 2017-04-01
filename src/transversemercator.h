#ifndef TRANSVERSEMERCATOR_H
#define TRANSVERSEMERCATOR_H

#include "projection.h"

class TransverseMercator : public Projection
{
public:
	TransverseMercator();
	TransverseMercator(double centralMeridian, double scale,
	  double falseEasting, double falseNorthing);

	virtual QPointF ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const QPointF &p) const;

private:
	double _centralMeridian;
	double _scale;
	double _falseEasting;
	double _falseNorthing;
};

#endif // TRANSVERSEMERCATOR_H
