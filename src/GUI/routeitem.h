#ifndef ROUTEITEM_H
#define ROUTEITEM_H

#include "data/link.h"
#include "pathitem.h"

class Map;
class Route;
class WaypointItem;

class RouteItem : public PathItem
{
	Q_OBJECT

public:
	RouteItem(const Route &route, Map *map, QGraphicsItem *parent = 0);

	void setMap(Map *map);
	void setDigitalZoom(int zoom);

	void showWaypoints(bool show);
	void showWaypointLabels(bool show);
	void showWaypointIcons(bool show);

	ToolTip info(bool extended) const;

private:
	QVector<WaypointItem*> _waypoints;
};

#endif // ROUTEITEM_H
