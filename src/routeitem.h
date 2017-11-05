#ifndef ROUTEITEM_H
#define ROUTEITEM_H

#include "pathitem.h"
#include "route.h"

class Map;

class RouteItem : public PathItem
{
	Q_OBJECT

public:
	RouteItem(const Route &route, Map *map, QGraphicsItem *parent = 0);

	void setMap(Map *map);

	void showWaypoints(bool show);
	void showWaypointLabels(bool show);

private:
	QString toolTip(Units units) const;
};

#endif // ROUTEITEM_H
