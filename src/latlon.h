#ifndef LATLON_H
#define LATLON_H

#include "projection.h"

class LatLon : public Projection
{
public:
	virtual QPointF ll2xy(const Coordinates &c) const
	  {return QPointF(c.lon(), c.lat());}
	virtual Coordinates xy2ll(const QPointF &p) const
	  {return Coordinates(p.x(), p.y());}
};

#endif // LATLON_H
