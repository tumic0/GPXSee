#ifndef MERCATOR_H
#define MERCATOR_H

#include "projection.h"

class Mercator : public Projection
{
public:
	virtual QPointF ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const QPointF &p) const;
};

#endif // MERCATOR_H
