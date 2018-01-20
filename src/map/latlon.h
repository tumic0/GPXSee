#ifndef LATLON_H
#define LATLON_H

#include "projection.h"
#include "angularunits.h"

class LatLon : public Projection
{
public:
	LatLon(const AngularUnits &au) : _au(au) {}

	virtual QPointF ll2xy(const Coordinates &c) const
	  {return QPointF(_au.fromDegrees(c.lon()), _au.fromDegrees(c.lat()));}
	virtual Coordinates xy2ll(const QPointF &p) const
	  {return Coordinates(_au.toDegrees(p.x()), _au.toDegrees(p.y()));}

private:
	AngularUnits _au;
};

#endif // LATLON_H
