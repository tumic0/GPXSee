#include <QPainter>
#include "data/waypoint.h"
#include "map/map.h"
#include "format.h"
#include "waypointitem.h"
#include "tooltip.h"
#include "routeitem.h"


QString RouteItem::info() const
{
	ToolTip tt;

	if (!_name.isEmpty())
		tt.insert(tr("Name"), _name);
	if (!_desc.isEmpty())
		tt.insert(tr("Description"), _desc);
	if (!_comment.isEmpty() && _comment != _desc)
		tt.insert(tr("Comment"), _comment);
	tt.insert(tr("Distance"), Format::distance(path().last().last().distance(),
	  _units));
	if (!_links.isEmpty()) {
		QString links;
		for (int i = 0; i < _links.size(); i++) {
			const Link &link = _links.at(i);
			links.append(QString("<a href=\"%0\">%1</a>").arg(link.URL(),
			  link.text().isEmpty() ? link.URL() : link.text()));
			if (i != _links.size() - 1)
				links.append("<br/>");
		}
		tt.insert(tr("Links"), links);
	}

	return tt.toString();
}

RouteItem::RouteItem(const Route &route, Map *map, QGraphicsItem *parent)
  : PathItem(route.path(), map, parent)
{
	const RouteData &waypoints = route.data();

	_waypoints.resize(waypoints.size());
	for (int i = 0; i < waypoints.size(); i++)
		_waypoints[i] = new WaypointItem(waypoints.at(i), map, this);

	_name = route.name();
	_desc = route.description();
	_comment = route.comment();
	_links = route.links();
}

void RouteItem::setMap(Map *map)
{
	for (int i = 0; i < _waypoints.count(); i++)
		_waypoints[i]->setMap(map);

	PathItem::setMap(map);
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
