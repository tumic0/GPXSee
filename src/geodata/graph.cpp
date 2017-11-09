#include "graph.h"

QDebug operator<<(QDebug dbg, const GraphPoint &point)
{
	dbg.nospace() << "GraphPoint(" << point.s() << ", " << point.t() << ", "
	  << point.y() << ")";
	return dbg.space();
}
