#include <QApplication>
#include <QPainter>
#include "ll.h"
#include "misc.h"
#include "waypoint.h"
#include "waypointitem.h"
#include "tooltip.h"
#include "routeitem.h"


QString RouteItem::toolTip()
{
	ToolTip tt;

	tt.insert(qApp->translate("RouteItem", "Distance"),
	  ::distance(_distance, _units));

	return tt.toString();
}

RouteItem::RouteItem(const Route &route, QGraphicsItem *parent)
  : PathItem(parent)
{
	QVector<Waypoint> r = route.route();
	Q_ASSERT(r.count() >= 2);

	new WaypointItem(r.at(0), this);
	const QPointF &p = r.at(0).coordinates();
	_path.moveTo(ll2mercator(QPointF(p.x(), -p.y())));
	for (int i = 1; i < r.size(); i++) {
		const QPointF &p = r.at(i).coordinates();
		_path.lineTo(ll2mercator(QPointF(p.x(), -p.y())));
		new WaypointItem(r.at(i), this);
	}

	updateShape();

	_distance = route.distance();

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
