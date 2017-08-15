#include "waypoint.h"

QDebug operator<<(QDebug dbg, const Waypoint &waypoint)
{
	dbg.nospace() << "Waypoint(" << waypoint.coordinates() << ", "
	  << waypoint.name() << ", " << waypoint.description() << ")";
	return dbg.space();
}
