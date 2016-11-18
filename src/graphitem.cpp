#include <QPainter>
#include "graphitem.h"


#define GRAPH_WIDTH 1
#define HOVER_WIDTH 2

GraphItem::GraphItem(const Graph &graph, QGraphicsItem *parent)
  : QGraphicsObject(parent)
{
	_id = 0;

	_pen = QPen(Qt::black, GRAPH_WIDTH);

	_type = Distance;
	_graph = graph;
	_sx = 1.0; _sy = 1.0;

	updatePath();
	updateBounds();
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
	prepareGeometryChange();

	_type = type;
	updatePath();
	updateBounds();
}

void GraphItem::setColor(const QColor &color)
{
	_pen.setColor(color);
	update();
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
		if (_graph.hasTime()) {
			if (pos >= _graph.first().t() && pos <= _graph.last().t())
				emit sliderPositionChanged(distanceAtTime(pos));
			else
				emit sliderPositionChanged(NAN);
		} else
			emit sliderPositionChanged(NAN);
	} else
		emit sliderPositionChanged(pos);
}

void GraphItem::selected(bool selected)
{
	if (selected) {
		_pen.setWidth(HOVER_WIDTH);
		setZValue(zValue() + 1.0);
	} else {
		_pen.setWidth(GRAPH_WIDTH);
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
	_path = QPainterPath();

	if (_type == Time && !_graph.hasTime())
		return;

	_path.moveTo(_graph.first().x(_type) * _sx, -_graph.first().y() * _sy);
	for (int i = 1; i < _graph.size(); i++)
		_path.lineTo(_graph.at(i).x(_type) * _sx, -_graph.at(i).y() * _sy);
}

void GraphItem::updateBounds()
{
	if (_type == Time && !_graph.hasTime()) {
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
