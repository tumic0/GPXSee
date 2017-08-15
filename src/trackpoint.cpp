#include "trackpoint.h"

QDebug operator<<(QDebug dbg, const Trackpoint &trackpoint)
{
	dbg.nospace() << "Trackpoint(" << trackpoint.coordinates() << ", "
	  << trackpoint.timestamp() << ", " << trackpoint.elevation() << ", "
	  << trackpoint.speed() << ", " << trackpoint.heartRate() << ", "
	  << trackpoint.temperature() << ")";
	return dbg.space();
}
