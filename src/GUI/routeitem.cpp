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
	tt.insert(tr("Distance"), Format::distance(path().last().last().distance(),
	  units));

	return tt.toString();
}

RouteItem::RouteItem(const Route &route, Map *map, QGraphicsItem *parent)
  : PathItem(route.path(), map, parent)
{
	const QVector<Waypoint> &waypoints = route.waypoints();

	_waypoints.resize(waypoints.size());
	for (int i = 0; i < waypoints.size(); i++)
		_waypoints[i] = new WaypointItem(waypoints.at(i), map, this);

	_name = route.name();
	_desc = route.description();
	_coordinatesFormat = DecimalDegrees;

	setToolTip(toolTip(Metric));
}

void RouteItem::setMap(Map *map)
{
	for (int i = 0; i < _waypoints.count(); i++)
		_waypoints[i]->setMap(map);

	PathItem::setMap(map);
}

void RouteItem::setUnits(Units u)
{
	if (units() == u)
		return;

	PathItem::setUnits(u);

	setToolTip(toolTip(units()));

	for (int i = 0; i < _waypoints.count(); i++)
		_waypoints[i]->setToolTipFormat(units(), _coordinatesFormat);
}

void RouteItem::setCoordinatesFormat(CoordinatesFormat format)
{
	if (_coordinatesFormat == format)
		return;

	_coordinatesFormat = format;

	for (int i = 0; i < _waypoints.count(); i++)
		_waypoints[i]->setToolTipFormat(units(), _coordinatesFormat);
}

void RouteItem::showWaypoints(bool show)
{
	for (int i = 0; i < _waypoints.count(); i++)
		_waypoints[i]->setVisible(show);
}

void RouteItem::showWaypointLabels(bool show)
{
	for (int i = 0; i < _waypoints.count(); i++)
		_waypoints[i]->showLabel(show);
}
