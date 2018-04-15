#ifndef LATLON_H
#define LATLON_H

#include "ct.h"
#include "angularunits.h"

class LatLon : public CT
{
public:
	LatLon(const AngularUnits &au) : _au(au) {}

	virtual CT *clone() const {return new LatLon(*this);}

	virtual PointD ll2xy(const Coordinates &c) const
	  {return PointD(_au.fromDegrees(c.lon()), _au.fromDegrees(c.lat()));}
	virtual Coordinates xy2ll(const PointD &p) const
	  {return Coordinates(_au.toDegrees(p.x()), _au.toDegrees(p.y()));}

private:
	AngularUnits _au;
};

#endif // LATLON_H
