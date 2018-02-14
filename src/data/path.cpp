#include "path.h"

RectC Path::boundingRect() const
{
	RectC ret;

	if (size() < 2)
		return ret;

	for (int i = 0; i < size(); i++)
		ret.unite(at(i).coordinates());

	return ret;
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const PathPoint &point)
{
	dbg.nospace() << "PathPoint(" << point.distance() << ", "
	  << point.coordinates() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
