#ifndef ROUTEITEM_H
#define ROUTEITEM_H

#include "data/route.h"
#include "pathitem.h"
#include "units.h"
#include "format.h"
#include "graphicsscene.h"

class Map;
class WaypointItem;

class RouteItem : public PathItem
{
	Q_OBJECT

public:
	RouteItem(const Route &route, Map *map, QGraphicsItem *parent = 0);

	void setMap(Map *map);

	void showWaypoints(bool show);
	void showWaypointLabels(bool show);

	QString info() const;

private:
	QString _name;
	QString _desc;
	QString _comment;
	QVector<Link> _links;

	QVector<WaypointItem*> _waypoints;
};

#endif // ROUTEITEM_H
