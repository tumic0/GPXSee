#include <QPainter>
#include "ll.h"
#include "waypoint.h"
#include "waypointitem.h"
#include "routeitem.h"


#define ROUTE_WIDTH     3

RouteItem::RouteItem(const Route &route, QGraphicsItem *parent)
  : QGraphicsItem(parent)
{
	WaypointItem *wi;

	const QVector<Waypoint> &r = route.route();
	Q_ASSERT(r.count() >= 2);

	wi = new WaypointItem(r.at(0));
	wi->setParentItem(this);
	const QPointF &p = r.at(0).coordinates();
	_path.moveTo(ll2mercator(QPointF(p.x(), -p.y())));
	for (int i = 1; i < r.size(); i++) {
		const QPointF &p = r.at(i).coordinates();
		_path.lineTo(ll2mercator(QPointF(p.x(), -p.y())));
		wi = new WaypointItem(r.at(i));
		wi->setParentItem(this);
	}

	QBrush brush(Qt::SolidPattern);
	_pen = QPen(brush, ROUTE_WIDTH, Qt::DotLine);

	_marker = new MarkerItem(this);
	_marker->setPos(_path.pointAtPercent(0));
}

void RouteItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	painter->setPen(_pen);
	painter->drawPath(_path);

/*
	painter->setPen(Qt::red);
	painter->drawRect(boundingRect());
*/
}

void RouteItem::setScale(qreal scale)
{
	prepareGeometryChange();

	_pen.setWidthF(ROUTE_WIDTH * 1.0/scale);
	QGraphicsItem::setScale(scale);

	QList<QGraphicsItem *> childs =	childItems();
	for (int i = 0; i < childs.count(); i++)
		childs.at(i)->setScale(1.0/scale);
}

void RouteItem::setColor(const QColor &color)
{
	_pen.setColor(color);
	update();
}

void RouteItem::moveMarker(qreal t)
{
	Q_ASSERT(t >= 0 && t <= 1.0);
	_marker->setPos(_path.pointAtPercent(t));
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
