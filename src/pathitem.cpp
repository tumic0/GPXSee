#include <cmath>
#include <QApplication>
#include <QCursor>
#include <QPainter>
#include "tooltip.h"
#include "map.h"
#include "misc.h"
#include "pathitem.h"


PathItem::PathItem(const Path &path, Map *map, QGraphicsItem *parent)
  : QGraphicsObject(parent)
{
	Q_ASSERT(path.count() >= 2);

	_path = path;
	_map = map;
	_digitalZoom = 0;

	_width = 3;
	QBrush brush(Qt::SolidPattern);
	_pen = QPen(brush, _width);

	updatePainterPath(map);
	updateShape();

	_marker = new MarkerItem(this);
	_marker->setPos(position(_path.at(0).distance()));
	_markerDistance = _path.at(0).distance();

	setCursor(Qt::ArrowCursor);
	setAcceptHoverEvents(true);
}

void PathItem::updateShape()
{
	QPainterPathStroker s;
	s.setWidth((_width + 1) * pow(2, -_digitalZoom));
	_shape = s.createStroke(_painterPath);
}

void PathItem::updatePainterPath(Map *map)
{
	_painterPath = QPainterPath();

	_painterPath.moveTo(map->ll2xy(_path.first().coordinates()));
	for (int i = 1; i < _path.size(); i++)
		_painterPath.lineTo(map->ll2xy(_path.at(i).coordinates()));
}

void PathItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	painter->setPen(_pen);
	painter->drawPath(_painterPath);

/*
	QPen p = QPen(QBrush(Qt::red), 0);
	painter->setPen(p);
	painter->drawRect(boundingRect());
*/
}

void PathItem::setMap(Map *map)
{
	prepareGeometryChange();

	_map = map;

	updatePainterPath(map);
	updateShape();

	_marker->setPos(position(_markerDistance));
}

void PathItem::setColor(const QColor &color)
{
	if (_pen.color() == color)
		return;

	_pen.setColor(color);
	update();
}

void PathItem::setWidth(qreal width)
{
	if (_width == width)
		return;

	prepareGeometryChange();

	_width = width;
	_pen.setWidthF(_width * pow(2, -_digitalZoom));

	updateShape();
}

void PathItem::setStyle(Qt::PenStyle style)
{
	if (_pen.style() == style)
		return;

	_pen.setStyle(style);
	update();
}

void PathItem::setDigitalZoom(int zoom)
{
	if (_digitalZoom == zoom)
		return;

	prepareGeometryChange();

	_digitalZoom = zoom;
	_pen.setWidthF(_width * pow(2, -_digitalZoom));
	_marker->setScale(pow(2, -_digitalZoom));

	updateShape();
}

QPointF PathItem::position(qreal x) const
{
	int low = 0;
	int high = _path.count() - 1;
	int mid = 0;

	Q_ASSERT(high > low);
	Q_ASSERT(x >= _path.at(low).distance() && x <= _path.at(high).distance());

	while (low <= high) {
		mid = low + ((high - low) / 2);
		qreal val = _path.at(mid).distance();
		if (val > x)
			high = mid - 1;
		else if (val < x)
			low = mid + 1;
		else
			return _map->ll2xy(_path.at(mid).coordinates());
	}

	QLineF l;
	qreal p1, p2;
	if (_path.at(mid).distance() < x) {
		l = QLineF(_map->ll2xy(_path.at(mid).coordinates()),
		  _map->ll2xy(_path.at(mid+1).coordinates()));
		p1 = _path.at(mid).distance(); p2 = _path.at(mid+1).distance();
	} else {
		l = QLineF(_map->ll2xy(_path.at(mid-1).coordinates()),
		  _map->ll2xy(_path.at(mid).coordinates()));
		p1 = _path.at(mid-1).distance(); p2 = _path.at(mid).distance();
	}

	return l.pointAt((x - p1) / (p2 - p1));
}

void PathItem::moveMarker(qreal distance)
{
	if (distance >= _path.first().distance()
	  && distance <= _path.last().distance()) {
		_marker->setVisible(true);
		_marker->setPos(position(distance));
		_markerDistance = distance;
	} else
		_marker->setVisible(false);
}

void PathItem::hover(bool hover)
{
	if (hover) {
		_pen.setWidth(_width + 1);
		setZValue(zValue() + 1.0);
	} else {
		_pen.setWidth(_width);
		setZValue(zValue() - 1.0);
	}

	update();
}

void PathItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_pen.setWidthF((_width + 1) * pow(2, -_digitalZoom));
	setZValue(zValue() + 1.0);
	update();

	emit selected(true);
}

void PathItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_pen.setWidthF(_width * pow(2, -_digitalZoom));
	setZValue(zValue() - 1.0);
	update();

	emit selected(false);
}
