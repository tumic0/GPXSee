#ifndef UTM_H
#define UTM_H

#include "projection.h"
#include "transversemercator.h"

class UTM : public Projection
{
public:
	UTM(int zone) : _tm((qAbs(zone) - 1)*6 - 180 + 3, 0.9996, 500000,
	  zone < 0 ? 10000000 : 0) {}

	virtual QPointF ll2xy(const Coordinates &c) const
	  {return _tm.ll2xy(c);}
	virtual Coordinates xy2ll(const QPointF &p) const
	  {return _tm.xy2ll(p);}

private:
	TransverseMercator _tm;
};

#endif // UTM_H
