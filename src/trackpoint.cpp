#include "trackpoint.h"

QDebug operator<<(QDebug dbg, const Trackpoint &trackpoint)
{
	const bool ais = dbg.autoInsertSpaces();
	dbg.nospace() << "Trackpoint(" << trackpoint.coordinates() << ", "
	  << trackpoint.timestamp() << ", " << trackpoint.elevation() << ", "
	  << trackpoint.speed() << ", " << trackpoint.heartRate() << ", "
	  << trackpoint.temperature() << ")";
	dbg.setAutoInsertSpaces(ais);
	return dbg.maybeSpace();
}
