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
	const bool ais = dbg.autoInsertSpaces();
	dbg.nospace() << "PathPoint(" << point.distance() << ", "
	  << point.coordinates() << ")";
	dbg.setAutoInsertSpaces(ais);
	return dbg.maybeSpace();
}
