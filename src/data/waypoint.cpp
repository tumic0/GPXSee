#include "dem.h"
#include "waypoint.h"

bool Waypoint::_useDEM = false;
bool Waypoint::_show2ndElevation = false;

QPair<qreal, qreal> Waypoint::elevations() const
{
	if (_useDEM) {
		qreal dem = DEM::elevation(coordinates());
		if (!std::isnan(dem))
			return QPair<qreal, qreal>(dem, _show2ndElevation ? elevation()
			  : NAN);
		else
			return QPair<qreal, qreal>(elevation(), NAN);
	} else {
		if (hasElevation())
			return QPair<qreal, qreal>(elevation(), _show2ndElevation
			  ? DEM::elevation(coordinates()) : NAN);
		else
			return QPair<qreal, qreal>(DEM::elevation(coordinates()), NAN);
	}
}
