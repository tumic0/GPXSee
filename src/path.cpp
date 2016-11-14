#include "path.h"

QDebug operator<<(QDebug dbg, const PathPoint &point)
{
	dbg.nospace() << "PathPoint(" << point.distance() << ", "
	  << point.coordinates() << ")";

	return dbg.maybeSpace();
}
