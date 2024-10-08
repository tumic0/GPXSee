#include <QLocale>
#include <QFileInfo>
#include "data/waypoint.h"
#include "data/route.h"
#include "format.h"
#include "waypointitem.h"
#include "tooltip.h"
#include "routeitem.h"


ToolTip RouteItem::info(bool extended) const
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
#ifdef Q_OS_ANDROID
	Q_UNUSED(extended);
#else // Q_OS_ANDROID
	if (extended && !_file.isEmpty())
		tt.insert(tr("File"), QString("<a href=\"file:%1\">%2</a>")
		  .arg(_file, QFileInfo(_file).fileName()));
#endif // Q_OS_ANDROID

	return tt;
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
	_file = route.file();
}

void RouteItem::setMap(Map *map)
{
	for (int i = 0; i < _waypoints.count(); i++)
		_waypoints.at(i)->setMap(map);

	PathItem::setMap(map);
}

void RouteItem::showWaypoints(bool show)
{
	for (int i = 0; i < _waypoints.count(); i++)
		_waypoints.at(i)->setVisible(show);
}

void RouteItem::showWaypointLabels(bool show)
{
	for (int i = 0; i < _waypoints.count(); i++)
		_waypoints.at(i)->showLabel(show);
}

void RouteItem::showWaypointIcons(bool show)
{
	for (int i = 0; i < _waypoints.count(); i++)
		_waypoints.at(i)->showIcon(show);
}

void RouteItem::setDigitalZoom(int zoom)
{
	for (int i = 0; i < _waypoints.count(); i++)
		_waypoints.at(i)->setDigitalZoom(zoom);

	PathItem::setDigitalZoom(zoom);
}
