#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include "popup.h"
#include "graphitem.h"


GraphItem::GraphItem(const Graph &graph, GraphType type, int width,
  const QColor &color, Qt::PenStyle style, QGraphicsItem *parent)
  : GraphicsItem(parent), _graph(graph), _type(type), _secondaryGraph(0)
{
	Q_ASSERT(_graph.isValid());

	_units = Metric;
	_pen = QPen(color, width, style, Qt::FlatCap);
	_sx = 0; _sy = 0;
	_time = _graph.hasTime();
	setZValue(2.0);
	setAcceptHoverEvents(true);

	updateBounds();
}

void GraphItem::updateShape()
{
	QPainterPathStroker s;
	s.setWidth(_pen.width() + 1);
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
	if (width == _pen.width())
		return;

	prepareGeometryChange();

	_pen.setWidth(width);

	updateShape();
}

const GraphSegment *GraphItem::segment(qreal x, GraphType type) const
{
	for (int i = 0; i < _graph.size(); i++)
		if (x <= _graph.at(i).last().x(type))
			return &(_graph.at(i));

	return 0;
}

qreal GraphItem::yAtX(qreal x) const
{
	const GraphSegment *seg = segment(x, _type);
	if (!seg)
		return NAN;

	int low = 0;
	int high = seg->count() - 1;
	int mid = 0;

	if (!(x >= seg->at(low).x(_type) && x <= seg->at(high).x(_type)))
		return NAN;

	while (low <= high) {
		mid = low + ((high - low) / 2);
		const GraphPoint &p = seg->at(mid);
		if (p.x(_type) > x)
			high = mid - 1;
		else if (p.x(_type) < x)
			low = mid + 1;
		else
			return p.y();
	}

	QLineF l;
	if (seg->at(mid).x(_type) < x)
		l = QLineF(seg->at(mid).x(_type), seg->at(mid).y(),
		  seg->at(mid+1).x(_type), seg->at(mid+1).y());
	else
		l = QLineF(seg->at(mid-1).x(_type), seg->at(mid-1).y(),
		  seg->at(mid).x(_type), seg->at(mid).y());

	return l.pointAt((x - l.p1().x()) / (l.p2().x() - l.p1().x())).y();
}

qreal GraphItem::distanceAtTime(qreal time) const
{
	if (!_time)
		return NAN;

	const GraphSegment *seg = segment(time, Time);
	if (!seg)
		return NAN;

	int low = 0;
	int high = seg->count() - 1;
	int mid = 0;

	if (!(time >= seg->at(low).t() && time <= seg->at(high).t()))
		return NAN;

	while (low <= high) {
		mid = low + ((high - low) / 2);
		const GraphPoint &p = seg->at(mid);
		if (p.t() > time)
			high = mid - 1;
		else if (p.t() < time)
			low = mid + 1;
		else
			return seg->at(mid).s();
	}

	QLineF l;
	if (seg->at(mid).t() < time)
		l = QLineF(seg->at(mid).t(), seg->at(mid).s(), seg->at(mid+1).t(),
		  seg->at(mid+1).s());
	else
		l = QLineF(seg->at(mid-1).t(), seg->at(mid-1).s(),
		  seg->at(mid).t(), seg->at(mid).s());

	return l.pointAt((time - l.p1().x()) / (l.p2().x() - l.p1().x())).y();
}

qreal GraphItem::timeAtDistance(qreal distance) const
{
	if (!_time)
		return NAN;

	const GraphSegment *seg = segment(distance, Distance);
	if (!seg)
		return NAN;

	int low = 0;
	int high = seg->count() - 1;
	int mid = 0;

	if (!(distance >= seg->at(low).s() && distance <= seg->at(high).s()))
		return NAN;

	while (low <= high) {
		mid = low + ((high - low) / 2);
		const GraphPoint &p = seg->at(mid);
		if (p.s() > distance)
			high = mid - 1;
		else if (p.s() < distance)
			low = mid + 1;
		else
			return seg->at(mid).t();
	}

	QLineF l;
	if (seg->at(mid).s() < distance)
		l = QLineF(seg->at(mid).s(), seg->at(mid).t(), seg->at(mid+1).s(),
		  seg->at(mid+1).t());
	else
		l = QLineF(seg->at(mid-1).s(), seg->at(mid-1).t(),
		  seg->at(mid).s(), seg->at(mid).t());

	return l.pointAt((distance - l.p1().x()) / (l.p2().x() - l.p1().x())).y();
}

void GraphItem::hover(bool hover)
{
	if (hover) {
		_pen.setWidth(_pen.width() + 1);
		setZValue(zValue() + 1.0);
	} else {
		_pen.setWidth(_pen.width() - 1);
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
}

void GraphItem::updatePath()
{
	if (_sx == 0 && _sy == 0)
		return;

	prepareGeometryChange();

	_path = QPainterPath();

	if (!(_type == Time && !_time)) {
		for (int i = 0; i < _graph.size(); i++) {
			const GraphSegment &segment = _graph.at(i);

			_path.moveTo(segment.first().x(_type) * _sx, -segment.first().y()
			  * _sy);
			for (int i = 1; i < segment.size(); i++)
				_path.lineTo(segment.at(i).x(_type) * _sx, -segment.at(i).y()
				  * _sy);
		}
	}

	updateShape();
}

void GraphItem::updateBounds()
{
	if (_type == Time && !_time) {
		_bounds = QRectF();
		return;
	}

	qreal bottom, top, left, right;

	QPointF p = QPointF(_graph.first().first().x(_type),
	  -_graph.first().first().y());
	bottom = p.y(); top = p.y(); left = p.x(); right = p.x();

	for (int i = 0; i < _graph.size(); i++) {
		const GraphSegment &segment = _graph.at(i);

		for (int j = 0; j < segment.size(); j++) {
			p = QPointF(segment.at(j).x(_type), -segment.at(j).y());
			bottom = qMax(bottom, p.y()); top = qMin(top, p.y());
			right = qMax(right, p.x()); left = qMin(left, p.x());
		}
	}

	if (left == right)
		_bounds = QRectF();
	else
		_bounds = QRectF(QPointF(left, top), QPointF(right, bottom));
}

qreal GraphItem::max() const
{
	qreal ret = _graph.first().first().y();

	for (int i = 0; i < _graph.size(); i++) {
		const GraphSegment &segment = _graph.at(i);

		for (int j = 0; j < segment.size(); j++) {
			qreal y = segment.at(j).y();
			if (y > ret)
				ret = y;
		}
	}

	return ret;
}

qreal GraphItem::min() const
{
	qreal ret = _graph.first().first().y();

	for (int i = 0; i < _graph.size(); i++) {
		const GraphSegment &segment = _graph.at(i);

		for (int j = 0; j < segment.size(); j++) {
			qreal y = segment.at(j).y();
			if (y < ret)
				ret = y;
		}
	}

	return ret;
}

qreal GraphItem::avg() const
{
	qreal sum = 0;

	for (int i = 0; i < _graph.size(); i++) {
		const GraphSegment &segment = _graph.at(i);

		for (int j = 1; j < segment.size(); j++)
			sum += segment.at(j).y() * (segment.at(j).s() - segment.at(j-1).s());
	}

	return sum/_graph.last().last().s();
}

void GraphItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_pen.setWidth(_pen.width() + 1);
	setZValue(zValue() + 1.0);
	update();

	emit selected(true);
}

void GraphItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_pen.setWidth(_pen.width() - 1);
	setZValue(zValue() - 1.0);
	update();

	emit selected(false);
}

void GraphItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	Popup::show(event->screenPos(), info(), event->widget());
	GraphicsItem::mousePressEvent(event);
}
