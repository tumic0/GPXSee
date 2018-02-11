#include <QPainter>
#include "data/waypoint.h"
#include "map/map.h"
#include "format.h"
#include "waypointitem.h"
#include "tooltip.h"
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
	_units = Metric;
	_coordinatesFormat = DecimalDegrees;

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

void RouteItem::setUnits(Units units)
{
	if (_units == units)
		return;

	_units = units;

	setToolTip(toolTip(_units));

	QList<QGraphicsItem *> childs =	childItems();
	for (int i = 0; i < childs.count(); i++) {
		if (childs.at(i) != _marker) {
			WaypointItem *wi = static_cast<WaypointItem*>(childs.at(i));
			wi->setToolTipFormat(_units, _coordinatesFormat);
		}
	}
}

void RouteItem::setCoordinatesFormat(CoordinatesFormat format)
{
	if (_coordinatesFormat == format)
		return;

	_coordinatesFormat = format;

	QList<QGraphicsItem *> childs =	childItems();
	for (int i = 0; i < childs.count(); i++) {
		if (childs.at(i) != _marker) {
			WaypointItem *wi = static_cast<WaypointItem*>(childs.at(i));
			wi->setToolTipFormat(_units, _coordinatesFormat);
		}
	}
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
