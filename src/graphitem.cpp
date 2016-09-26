#include <QPainter>
#include "graphitem.h"


#define GRAPH_WIDTH 1
#define HOVER_WIDTH 2

static qreal yAtX(const QPainterPath &path, qreal x)
{
	int low = 0;
	int high = path.elementCount() - 1;
	int mid = 0;

	Q_ASSERT(high > low);
	Q_ASSERT(x >= path.elementAt(low).x && x <= path.elementAt(high).x);

	while (low <= high) {
		mid = low + ((high - low) / 2);
		const QPainterPath::Element &e = path.elementAt(mid);
		if (e.x > x)
			high = mid - 1;
		else if (e.x < x)
			low = mid + 1;
		else
			return e.y;
	}

	QLineF l;
	if (path.elementAt(mid).x < x)
		l = QLineF(path.elementAt(mid).x, path.elementAt(mid).y,
		  path.elementAt(mid+1).x, path.elementAt(mid+1).y);
	else
		l = QLineF(path.elementAt(mid-1).x, path.elementAt(mid-1).y,
		  path.elementAt(mid).x, path.elementAt(mid).y);

	return l.pointAt((x - l.p1().x()) / (l.p2().x() - l.p1().x())).y();
}

static bool hasTime(const Graph &graph)
{
	for (int i = 0; i < graph.count(); i++)
		if (std::isnan(graph.at(i).t()))
			return false;

	return true;
}

GraphItem::GraphItem(const Graph &graph, QGraphicsItem *parent)
  : QGraphicsObject(parent)
{
	_id = 0;
	_type = Distance;

	_pen = QPen(Qt::black, GRAPH_WIDTH);
	_pen.setCosmetic(true);

	_distancePath.moveTo(graph.first().s(), -graph.first().y());
	for (int i = 1; i < graph.size(); i++)
		_distancePath.lineTo(graph.at(i).s(), -graph.at(i).y());

	if (hasTime(graph)) {
		_timePath.moveTo(graph.first().t(), -graph.first().y());
		for (int i = 1; i < graph.size(); i++)
			_timePath.lineTo(graph.at(i).t(), -graph.at(i).y());
	}
}

void GraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	painter->setPen(_pen);
	painter->drawPath((_type == Distance) ? _distancePath : _timePath);

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
}

void GraphItem::setColor(const QColor &color)
{
	_pen.setColor(color);
	update();
}

qreal GraphItem::yAtX(qreal x)
{
	return ::yAtX((_type == Distance) ? _distancePath : _timePath, x);
}

qreal GraphItem::distanceAtTime(qreal time)
{
	int low = 0;
	int high = _timePath.elementCount() - 1;
	int mid = 0;

	Q_ASSERT(high > low);
	Q_ASSERT(time >= _timePath.elementAt(low).x
	  && time <= _timePath.elementAt(high).x);

	while (low <= high) {
		mid = low + ((high - low) / 2);
		const QPainterPath::Element &e = _timePath.elementAt(mid);
		if (e.x > time)
			high = mid - 1;
		else if (e.x < time)
			low = mid + 1;
		else
			return _distancePath.elementAt(mid).x;
	}

	QLineF l;
	if (_timePath.elementAt(mid).x < time)
		l = QLineF(_timePath.elementAt(mid).x, _distancePath.elementAt(mid).x,
		  _timePath.elementAt(mid+1).x, _distancePath.elementAt(mid+1).x);
	else
		l = QLineF(_timePath.elementAt(mid-1).x, _distancePath.elementAt(mid-1).x,
		  _timePath.elementAt(mid).x, _distancePath.elementAt(mid).x);

	return l.pointAt((time - l.p1().x()) / (l.p2().x() - l.p1().x())).y();
}

void GraphItem::emitSliderPositionChanged(qreal pos)
{
	if (_type == Time) {
		if (!_timePath.isEmpty()) {
			if (pos <= _timePath.elementAt(_timePath.elementCount() - 1).x)
				emit sliderPositionChanged(distanceAtTime(pos));
			else
				emit sliderPositionChanged(_distancePath.elementAt(
			      _distancePath.elementCount() - 1).x + 1);
		} else
			emit sliderPositionChanged(_distancePath.elementAt(
		      _distancePath.elementCount() - 1).x + 1);
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
