#include <cmath>
#include <QCursor>
#include <QPainter>
#include "common/greatcircle.h"
#include "map/map.h"
#include "pathitem.h"


#define GEOGRAPHICAL_MILE 1855.3248

static unsigned segments(qreal distance)
{
	return ceil(distance / GEOGRAPHICAL_MILE);
}

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

	updatePainterPath();
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

void PathItem::addSegment(const Coordinates &c1, const Coordinates &c2)
{
	if (fabs(c1.lon() - c2.lon()) > 180.0) {
		// Split segment on date line crossing
		QPointF p;

		if (c2.lon() < 0) {
			QLineF l(QPointF(c1.lon(), c1.lat()), QPointF(c2.lon() + 360,
			  c2.lat()));
			QLineF dl(QPointF(180, -90), QPointF(180, 90));
			l.intersect(dl, &p);
			_painterPath.lineTo(_map->ll2xy(Coordinates(180, p.y())));
			_painterPath.moveTo(_map->ll2xy(Coordinates(-180, p.y())));
		} else {
			QLineF l(QPointF(c1.lon(), c1.lat()), QPointF(c2.lon() - 360,
			  c2.lat()));
			QLineF dl(QPointF(-180, -90), QPointF(-180, 90));
			l.intersect(dl, &p);
			_painterPath.lineTo(_map->ll2xy(Coordinates(-180, p.y())));
			_painterPath.moveTo(_map->ll2xy(Coordinates(180, p.y())));
		}
		_painterPath.lineTo(_map->ll2xy(c2));
	} else
		_painterPath.lineTo(_map->ll2xy(c2));
}

void PathItem::updatePainterPath()
{
	_painterPath = QPainterPath();

	_painterPath.moveTo(_map->ll2xy(_path.first().coordinates()));
	for (int i = 1; i < _path.size(); i++) {
		const PathPoint &p1 = _path.at(i-1);
		const PathPoint &p2 = _path.at(i);
		unsigned n = segments(p2.distance() - p1.distance());

		if (n > 1) {
			GreatCircle gc(p1.coordinates(), p2.coordinates());
			Coordinates last = p1.coordinates();

			for (unsigned j = 1; j <= n; j++) {
				Coordinates c(gc.pointAt(j/(double)n));
				addSegment(last, c);
				last = c;
			}
		} else
			addSegment(p1.coordinates(), p2.coordinates());
	}
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

	updatePainterPath();
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

	Coordinates c1, c2;
	qreal p1, p2;

	if (_path.at(mid).distance() < x) {
		c1 = _path.at(mid).coordinates(); c2 = _path.at(mid+1).coordinates();
		p1 = _path.at(mid).distance(); p2 = _path.at(mid+1).distance();
	} else {
		c1 = _path.at(mid-1).coordinates(); c2 = _path.at(mid).coordinates();
		p1 = _path.at(mid-1).distance(); p2 = _path.at(mid).distance();
	}

	unsigned n = segments(p2 - p1);
	if (n > 1) {
		GreatCircle gc(c1, c2);

		// Great circle point
		double f = (x - p1) / (p2 - p1);
		QPointF p(_map->ll2xy(gc.pointAt(f)));

		// Segment line of the great circle path
		double f1 = floor(n * f) / n;
		double f2 = ceil(n * f) / n;
		QLineF l(_map->ll2xy(gc.pointAt(f1)), _map->ll2xy(gc.pointAt(f2)));

		// Project the great circle point to the segment line
		QLineF u = l.unitVector();
		double lambda = (u.dx() * (p.x() - l.p1().x())) + (u.dy() * (p.y()
		  - l.p1().y()));
		return QPointF((u.dx() * lambda) + l.p1().x(), (u.dy() * lambda)
		  + l.p1().y());
	} else {
		QLineF l(_map->ll2xy(c1), _map->ll2xy(c2));
		return l.pointAt((x - p1) / (p2 - p1));
	}
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

void PathItem::setMarkerColor(const QColor &color)
{
	_marker->setColor(color);
}

void PathItem::hover(bool hover)
{
	if (hover) {
		_pen.setWidth((_width + 1) * pow(2, -_digitalZoom));
		setZValue(zValue() + 1.0);
	} else {
		_pen.setWidth(_width * pow(2, -_digitalZoom));
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
