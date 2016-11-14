#include <QApplication>
#include <QPainter>
#include "format.h"
#include "waypoint.h"
#include "waypointitem.h"
#include "tooltip.h"
#include "routeitem.h"


QString RouteItem::toolTip()
{
	ToolTip tt;

	if (!_name.isEmpty())
		tt.insert(qApp->translate("RouteItem", "Name"), _name);
	if (!_desc.isEmpty())
		tt.insert(qApp->translate("RouteItem", "Description"), _desc);
	tt.insert(qApp->translate("RouteItem", "Distance"),
	  Format::distance(_distance.last(), _units));

	return tt.toString();
}

RouteItem::RouteItem(const Route &route, QGraphicsItem *parent)
  : PathItem(parent)
{
	const RouteData &r = route.routeData();
	Q_ASSERT(r.count() >= 2);
	QPointF p;

	_name = r.name();
	_desc = r.description();
	_distance = route.distanceData();

	new WaypointItem(r.at(0), this);
	p = r.at(0).coordinates().toMercator();
	_path.moveTo(QPointF(p.x(), -p.y()));
	for (int i = 1; i < r.size(); i++) {
		p = r.at(i).coordinates().toMercator();
		_path.lineTo(QPointF(p.x(), -p.y()));
		new WaypointItem(r.at(i), this);
	}

	updateShape();

	_marker->setPos(_path.pointAtPercent(0));

	_pen.setStyle(Qt::DotLine);

	setToolTip(toolTip());
}

void RouteItem::setScale(qreal scale)
{
	QList<QGraphicsItem *> childs =	childItems();
	for (int i = 0; i < childs.count(); i++)
		childs.at(i)->setScale(1.0/scale);

	PathItem::setScale(scale);
}

void RouteItem::setUnits(enum Units units)
{
	PathItem::setUnits(units);
	setToolTip(toolTip());
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
