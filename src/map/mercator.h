#ifndef MERCATOR_H
#define MERCATOR_H

#include "ct.h"

class Mercator : public CT
{
public:
	virtual CT *clone() const {return new Mercator(*this);}

	virtual QPointF ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const QPointF &p) const;
};

#endif // MERCATOR_H
