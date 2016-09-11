#include <QApplication>
#include <QPainter>
#include "ll.h"
#include "misc.h"
#include "waypoint.h"
#include "waypointitem.h"
#include "tooltip.h"
#include "routeitem.h"


#define ROUTE_WIDTH 3
#define HOVER_WIDTH 5

QString RouteItem::toolTip()
{
	ToolTip tt;

	tt.insert(qApp->translate("RouteItem", "Distance"),
	  ::distance(_distance, _units));

	return tt.toString();
}

void RouteItem::updateShape()
{
	QPainterPathStroker s;
	s.setWidth(HOVER_WIDTH * 1.0/scale());
	_shape = s.createStroke(_path);
}

RouteItem::RouteItem(const Route &route, QGraphicsItem *parent)
  : QGraphicsItem(parent)
{
	WaypointItem *wi;

	QVector<Waypoint> r = route.route();
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

	_units = Metric;
	_distance = route.distance();

	setToolTip(toolTip());
	setCursor(Qt::ArrowCursor);
	setAcceptHoverEvents(true);
	setCacheMode(QGraphicsItem::DeviceCoordinateCache);

	updateShape();

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
	QPen p = QPen(Qt::red);
	p.setWidthF(1.0/scale());
	painter->setPen(p);
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

	updateShape();
}

void RouteItem::setColor(const QColor &color)
{
	_pen.setColor(color);
	update();
}

void RouteItem::setUnits(enum Units units)
{
	_units = units;
	setToolTip(toolTip());
}

void RouteItem::moveMarker(qreal distance)
{
	if (distance > _distance)
		_marker->setVisible(false);
	else {
		_marker->setVisible(true);
		_marker->setPos(_path.pointAtPercent(distance / _distance));
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

void RouteItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_pen.setWidthF(HOVER_WIDTH * 1.0/scale());
	setZValue(3.0);
	update();
}

void RouteItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_pen.setWidthF(ROUTE_WIDTH * 1.0/scale());
	setZValue(0);
	update();
}
