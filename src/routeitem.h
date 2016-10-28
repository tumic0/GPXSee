#ifndef ROUTEITEM_H
#define ROUTEITEM_H

#include "pathitem.h"
#include "route.h"


class RouteItem : public PathItem
{
	Q_OBJECT

public:
	RouteItem(const Route &route, QGraphicsItem *parent = 0);

	void setUnits(enum Units units);
	void setScale(qreal scale);

	void showWaypoints(bool show);
	void showWaypointLabels(bool show);

private:
	QString toolTip();

	QString _name;
	QString _desc;
};

#endif // ROUTEITEM_H
