#include <QPainter>
#include "graphitem.h"


GraphItem::GraphItem(const Graph &graph, GraphType type, QGraphicsItem *parent)
  : QGraphicsObject(parent)
{
	_id = 0;
	_width = 1;

	_pen = QPen(Qt::black, _width);

	_type = type;
	_graph = graph;
	_sx = 1.0; _sy = 1.0;

	_time = true;
	for (int i = 0; i < _graph.size(); i++) {
		if (std::isnan(_graph.at(i).t())) {
			_time = false;
			break;
		}
	}

	setZValue(1.0);

	updatePath();
	updateShape();
	updateBounds();

	setAcceptHoverEvents(true);
}

void GraphItem::updateShape()
{
	QPainterPathStroker s;
	s.setWidth(_width + 1);
	_shape = s.createStroke(_path);
}

void GraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
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

void GraphItem::setGraphType(GraphType type)
{
	if (type == _type)
		return;

	prepareGeometryChange();

	_type = type;
	updatePath();
	updateShape();
	updateBounds();
}

void GraphItem::setColor(const QColor &color)
{
	if (_pen.color() == color)
		return;

	_pen.setColor(color);
	update();
}

void GraphItem::setWidth(int width)
{
	if (width == _width)
		return;

	prepareGeometryChange();

	_width = width;
	_pen.setWidth(width);

	updateShape();
}

qreal GraphItem::yAtX(qreal x)
{
	int low = 0;
	int high = _graph.count() - 1;
	int mid = 0;

	Q_ASSERT(high > low);
	Q_ASSERT(x >= _graph.at(low).x(_type) && x <= _graph.at(high).x(_type));

	while (low <= high) {
		mid = low + ((high - low) / 2);
		const GraphPoint &p = _graph.at(mid);
		if (p.x(_type) > x)
			high = mid - 1;
		else if (p.x(_type) < x)
			low = mid + 1;
		else
			return -p.y();
	}

	QLineF l;
	if (_graph.at(mid).x(_type) < x)
		l = QLineF(_graph.at(mid).x(_type), _graph.at(mid).y(),
		  _graph.at(mid+1).x(_type), _graph.at(mid+1).y());
	else
		l = QLineF(_graph.at(mid-1).x(_type), _graph.at(mid-1).y(),
		  _graph.at(mid).x(_type), _graph.at(mid).y());

	return -l.pointAt((x - l.p1().x()) / (l.p2().x() - l.p1().x())).y();
}

qreal GraphItem::distanceAtTime(qreal time)
{
	int low = 0;
	int high = _graph.count() - 1;
	int mid = 0;

	Q_ASSERT(high > low);
	Q_ASSERT(time >= _graph.at(low).t() && time <= _graph.at(high).t());

	while (low <= high) {
		mid = low + ((high - low) / 2);
		const GraphPoint &p = _graph.at(mid);
		if (p.t() > time)
			high = mid - 1;
		else if (p.t() < time)
			low = mid + 1;
		else
			return _graph.at(mid).s();
	}

	QLineF l;
	if (_graph.at(mid).t() < time)
		l = QLineF(_graph.at(mid).t(), _graph.at(mid).s(), _graph.at(mid+1).t(),
		  _graph.at(mid+1).s());
	else
		l = QLineF(_graph.at(mid-1).t(), _graph.at(mid-1).s(),
		  _graph.at(mid).t(), _graph.at(mid).s());

	return l.pointAt((time - l.p1().x()) / (l.p2().x() - l.p1().x())).y();
}

void GraphItem::emitSliderPositionChanged(qreal pos)
{
	if (_type == Time) {
		if (_time) {
			if (pos >= _graph.first().t() && pos <= _graph.last().t())
				emit sliderPositionChanged(distanceAtTime(pos));
			else
				emit sliderPositionChanged(NAN);
		} else
			emit sliderPositionChanged(NAN);
	} else
		emit sliderPositionChanged(pos);
}

void GraphItem::hover(bool hover)
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

void GraphItem::setScale(qreal sx, qreal sy)
{
	if (_sx == sx && _sy == sy)
		return;

	prepareGeometryChange();

	_sx = sx; _sy = sy;
	updatePath();
	updateShape();
}

void GraphItem::updatePath()
{
	_path = QPainterPath();

	if (_type == Time && !_time)
		return;

	_path.moveTo(_graph.first().x(_type) * _sx, -_graph.first().y() * _sy);
	for (int i = 1; i < _graph.size(); i++)
		_path.lineTo(_graph.at(i).x(_type) * _sx, -_graph.at(i).y() * _sy);
}

void GraphItem::updateBounds()
{
	if (_type == Time && !_time) {
		_bounds = QRectF();
		return;
	}

	qreal bottom, top, left, right;

	QPointF p = QPointF(_graph.first().x(_type), -_graph.first().y());
	bottom = p.y(); top = p.y(); left = p.x(); right = p.x();

	for (int i = 1; i < _graph.size(); i++) {
		p = QPointF(_graph.at(i).x(_type), -_graph.at(i).y());
		bottom = qMax(bottom, p.y()); top = qMin(top, p.y());
		right = qMax(right, p.x()); left = qMin(left, p.x());
	}

	_bounds = QRectF(QPointF(left, top), QPointF(right, bottom));
}

void GraphItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_pen.setWidthF(_width + 1);
	setZValue(zValue() + 1.0);
	update();

	emit selected(true);
}

void GraphItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_pen.setWidthF(_width);
	setZValue(zValue() - 1.0);
	update();

	emit selected(false);
}
