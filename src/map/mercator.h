#ifndef MERCATOR_H
#define MERCATOR_H

#include "ct.h"

class Mercator : public CT
{
public:
	virtual CT *clone() const {return new Mercator(*this);}

	virtual PointD ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const PointD &p) const;
};

#endif // MERCATOR_H
