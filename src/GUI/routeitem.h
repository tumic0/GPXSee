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

	ToolTip info() const;
	QDateTime date() const {return QDateTime();}

private:
	QString _name;
	QString _desc;
	QString _comment;
	QVector<Link> _links;

	QVector<WaypointItem*> _waypoints;
};

#endif // ROUTEITEM_H
