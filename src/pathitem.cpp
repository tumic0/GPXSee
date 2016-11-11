#include <QApplication>
#include <QCursor>
#include <QPainter>
#include "tooltip.h"
#include "pathitem.h"


#define PATH_WIDTH  3
#define HOVER_WIDTH 4

void PathItem::updateShape()
{
	QPainterPathStroker s;
	s.setWidth(HOVER_WIDTH * 1.0/scale());
	_shape = s.createStroke(_path);

	if (qMax(boundingRect().width(), boundingRect().height()) * scale() <= 768)
		setCacheMode(QGraphicsItem::DeviceCoordinateCache);
	else
		setCacheMode(QGraphicsItem::NoCache);
}

PathItem::PathItem(QGraphicsItem *parent) : QGraphicsObject(parent)
{
	QBrush brush(Qt::SolidPattern);
	_pen = QPen(brush, PATH_WIDTH);

	_units = Metric;
	_distance = 0;

	_marker = new MarkerItem(this);

	setCursor(Qt::ArrowCursor);
	setAcceptHoverEvents(true);
}

void PathItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	painter->setPen(_pen);
	painter->drawPath(_path);

/*
	QPen p = QPen(QBrush(Qt::red), 0);
	painter->setPen(p);
	painter->drawRect(boundingRect());
*/
}

void PathItem::setScale(qreal scale)
{
	prepareGeometryChange();

	_pen.setWidthF(PATH_WIDTH * 1.0/scale);
	QGraphicsItem::setScale(scale);
	_marker->setScale(1.0/scale);

	updateShape();
}

void PathItem::setColor(const QColor &color)
{
	_pen.setColor(color);
	update();
}

void PathItem::setUnits(enum Units units)
{
	_units = units;
}

void PathItem::moveMarker(qreal distance)
{
	if (distance > _distance)
		_marker->setVisible(false);
	else {
		_marker->setVisible(true);
		_marker->setPos(_path.pointAtPercent(distance / _distance));
	}
}

void PathItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_pen.setWidthF(HOVER_WIDTH * 1.0/scale());
	setZValue(zValue() + 1.0);
	update();

	emit selected(true);
}

void PathItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_pen.setWidthF(PATH_WIDTH * 1.0/scale());
	setZValue(zValue() - 1.0);
	update();

	emit selected(false);
}
