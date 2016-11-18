#include <cmath>
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

QPointF PathItem::position(qreal x) const
{
	int low = 0;
	int high = _distance.count() - 1;
	int mid = 0;


	Q_ASSERT(_distance.count() == _path.elementCount());
	Q_ASSERT(high > low);
	Q_ASSERT(x >= _distance.at(low) && x <= _distance.at(high));

	while (low <= high) {
		mid = low + ((high - low) / 2);
		qreal val = _distance.at(mid);
		if (val > x)
			high = mid - 1;
		else if (val < x)
			low = mid + 1;
		else
			return _path.elementAt(mid);
	}

	QLineF l;
	qreal p1, p2;
	if (_distance.at(mid) < x) {
		l = QLineF(_path.elementAt(mid), _path.elementAt(mid+1));
		p1 = _distance.at(mid); p2 = _distance.at(mid+1);
	} else {
		l = QLineF(_path.elementAt(mid-1), _path.elementAt(mid));
		p1 = _distance.at(mid-1); p2 = _distance.at(mid);
	}

	return l.pointAt((x - p1) / (p2 - p1));
}

void PathItem::moveMarker(qreal distance)
{
	if (distance >= _distance.first() && distance <= _distance.last()) {
		_marker->setVisible(true);
		_marker->setPos(position(distance));
	} else
		_marker->setVisible(false);
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
