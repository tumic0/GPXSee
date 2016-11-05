#include "graph.h"

QDebug operator<<(QDebug dbg, const GraphPoint &graphpoint)
{
	dbg.nospace() << "GraphPoint(" << graphpoint.s() << ", "
	  << graphpoint.t() << ", " << graphpoint.y() << ")";

	return dbg.maybeSpace();
}
