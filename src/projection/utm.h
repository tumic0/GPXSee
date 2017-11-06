#ifndef UTM_H
#define UTM_H

#include "projection.h"
#include "transversemercator.h"

class UTM : public Projection
{
public:
	UTM(const Ellipsoid &ellipsoid, int zone);
	UTM(const Ellipsoid &ellipsoid, const Coordinates &c);

	virtual QPointF ll2xy(const Coordinates &c) const
	  {return _tm.ll2xy(c);}
	virtual Coordinates xy2ll(const QPointF &p) const
	  {return _tm.xy2ll(p);}

private:
	TransverseMercator _tm;
};

#endif // UTM_H
