#include "graph.h"

QDebug operator<<(QDebug dbg, const GraphPoint &point)
{
	const bool ais = dbg.autoInsertSpaces();
	dbg.nospace() << "GraphPoint(" << point.s() << ", " << point.t() << ", "
	  << point.y() << ")";
	dbg.setAutoInsertSpaces(ais);
	return dbg.maybeSpace();
}
