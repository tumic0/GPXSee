#ifndef CT_H
#define CT_H

#include "common/coordinates.h"
#include "pointd.h"

class CT {
public:
	virtual ~CT() {}

	virtual CT *clone() const = 0;

	virtual PointD ll2xy(const Coordinates &c) const = 0;
	virtual Coordinates xy2ll(const PointD &p) const = 0;
};

#endif // CT_H
