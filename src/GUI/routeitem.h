#ifndef ROUTEITEM_H
#define ROUTEITEM_H

#include "data/route.h"
#include "pathitem.h"
#include "units.h"
#include "format.h"

class Map;

class RouteItem : public PathItem
{
	Q_OBJECT

public:
	RouteItem(const Route &route, Map *map, QGraphicsItem *parent = 0);

	void setMap(Map *map);

	void setUnits(Units units);
	void setCoordinatesFormat(CoordinatesFormat format);
	void showWaypoints(bool show);
	void showWaypointLabels(bool show);

private:
	QString toolTip(Units units) const;

	QString _name;
	QString _desc;
	Units _units;
	CoordinatesFormat _coordinatesFormat;
};

#endif // ROUTEITEM_H
