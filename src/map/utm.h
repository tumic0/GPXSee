#ifndef UTM_H
#define UTM_H

#include "projection.h"
#include "transversemercator.h"

class UTM : public Projection
{
public:
	UTM(const Ellipsoid &ellipsoid, int zone);

	virtual QPointF ll2xy(const Coordinates &c) const
	  {return _tm.ll2xy(c);}
	virtual Coordinates xy2ll(const QPointF &p) const
	  {return _tm.xy2ll(p);}

	static int zone(const Coordinates &c);

private:
	TransverseMercator _tm;
};

#endif // UTM_H
