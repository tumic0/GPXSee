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
	const QVector<qreal> &d = route.distanceData();
	QPointF p;


	Q_ASSERT(r.count() >= 2);
	Q_ASSERT(r.size() == d.size());

	_name = r.name();
	_desc = r.description();

	new WaypointItem(r.first(), this);
	p = r.first().coordinates().toMercator();
	_path.moveTo(QPointF(p.x(), -p.y()));
	_distance.append(d.first());
	for (int i = 1; i < r.size(); i++) {
		if (r.at(i).coordinates() == r.at(i-1).coordinates())
			continue;
		p = r.at(i).coordinates().toMercator();
		_path.lineTo(QPointF(p.x(), -p.y()));
		_distance.append(d.at(i));
		new WaypointItem(r.at(i), this);
	}

	updateShape();

	_marker->setPos(_path.elementAt(0));

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
