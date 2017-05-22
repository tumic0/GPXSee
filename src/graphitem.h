#ifndef GRAPHITEM_H
#define GRAPHITEM_H

#include <QGraphicsObject>
#include <QPen>
#include "graph.h"

class GraphItem : public QGraphicsObject
{
	Q_OBJECT

public:
	GraphItem(const Graph &graph, QGraphicsItem *parent = 0);

	QRectF boundingRect() const
	  {return _path.boundingRect();}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	const QRectF &bounds() const {return _bounds;}
	void setScale(qreal sx, qreal sy);

	void setGraphType(GraphType type);
	int id() const {return _id;}
	void setId(int id) {_id = id;}
	void setColor(const QColor &color);
	void setWidth(int width);

	qreal yAtX(qreal x);
	qreal distanceAtTime(qreal time);

signals:
	void sliderPositionChanged(qreal);

public slots:
	void emitSliderPositionChanged(qreal);
	void selected(bool selected);

private:
	void updatePath();
	void updateBounds();

	int _id;
	QPen _pen;
	int _width;

	Graph _graph;
	GraphType _type;

	QPainterPath _path;
	QRectF _bounds;
	qreal _sx, _sy;

	bool _time;
};

#endif // GRAPHITEM_H
