#include <QPainter>
#include "format.h"
#include "waypoint.h"
#include "waypointitem.h"
#include "tooltip.h"
#include "map.h"
#include "routeitem.h"


QString RouteItem::toolTip(Units units) const
{
	ToolTip tt;

	if (!_name.isEmpty())
		tt.insert(tr("Name"), _name);
	if (!_desc.isEmpty())
		tt.insert(tr("Description"), _desc);
	tt.insert(tr("Distance"), Format::distance(_path.last().distance(), units));

	return tt.toString();
}

RouteItem::RouteItem(const Route &route, Map *map, QGraphicsItem *parent)
  : PathItem(route.path(), map, parent)
{
	const QVector<Waypoint> &waypoints = route.waypoints();

	for (int i = 0; i < waypoints.size(); i++)
		new WaypointItem(waypoints.at(i), map, this);

	_name = route.name();
	_desc = route.description();

	setToolTip(toolTip(Metric));
}

void RouteItem::setMap(Map *map)
{
	QList<QGraphicsItem *> childs =	childItems();
	for (int i = 0; i < childs.count(); i++) {
		if (childs.at(i) != _marker) {
			WaypointItem *wi = static_cast<WaypointItem*>(childs.at(i));
			wi->setMap(map);
		}
	}

	PathItem::setMap(map);
}

void RouteItem::setUnits(enum Units units)
{
	setToolTip(toolTip(units));
}

void RouteItem::showWaypoints(bool show)
{
	QList<QGraphicsItem *> childs =	childItems();
	for (int i = 0; i < childs.count(); i++)
		if (childs.at(i) != _marker)
			childs.at(i)->setVisible(show);
}

void RouteItem::showWaypointLabels(bool show)
{
	QList<QGraphicsItem *> childs =	childItems();
	for (int i = 0; i < childs.count(); i++) {
		if (childs.at(i) != _marker) {
			WaypointItem *wi = static_cast<WaypointItem*>(childs.at(i));
			wi->showLabel(show);
		}
	}
}
