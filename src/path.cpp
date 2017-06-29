#include "path.h"

RectC Path::boundingRect() const
{
	if (size() < 2)
		return RectC();

	RectC ret(first().coordinates(), first().coordinates());
	for (int i = 1; i < size(); i++)
		ret.unite(at(i).coordinates());

	return ret;
}

QDebug operator<<(QDebug dbg, const PathPoint &point)
{
	dbg.nospace() << "PathPoint(" << point.distance() << ", "
	  << point.coordinates() << ")";

	return dbg.maybeSpace();
}
