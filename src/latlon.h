#ifndef LATLON_H
#define LATLON_H

#include "projection.h"

class LatLon : public Projection
{
public:
	virtual QPointF ll2xy(const Coordinates &c) const
	  {return c.toPointF();}
	virtual Coordinates xy2ll(const QPointF &p) const
	  {return Coordinates(p);}
};

#endif // LATLON_H
