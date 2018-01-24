#ifndef CT_H
#define CT_H

#include <QPointF>
#include "common/coordinates.h"

class CT {
public:
	virtual ~CT() {}

	virtual CT *clone() const = 0;

	virtual QPointF ll2xy(const Coordinates &c) const = 0;
	virtual Coordinates xy2ll(const QPointF &p) const = 0;
};

#endif // CT_H
