#ifndef PROJECTION_H
#define PROJECTION_H

#include <QPointF>
#include "coordinates.h"

class Projection {
public:
	virtual ~Projection() {}

	virtual QPointF ll2xy(const Coordinates &c) const = 0;
	virtual Coordinates xy2ll(const QPointF &p) const = 0;
};

#endif // PROJECTION_H
