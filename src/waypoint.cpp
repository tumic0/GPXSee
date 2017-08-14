#include "waypoint.h"

QDebug operator<<(QDebug dbg, const Waypoint &waypoint)
{
	const bool ais = dbg.autoInsertSpaces();
	dbg.nospace() << "Waypoint(" << waypoint.coordinates() << ", "
	  << waypoint.name() << ", " << waypoint.description() << ")";
	dbg.setAutoInsertSpaces(ais);
	return dbg.maybeSpace();
}
