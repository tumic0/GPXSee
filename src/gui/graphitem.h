#ifndef GRAPHITEM_H
#define GRAPHITEM_H

#include <QGraphicsObject>
#include <QPen>
#include "units.h"
#include "graph.h"

class GraphItem : public QGraphicsObject
{
	Q_OBJECT

public:
	GraphItem(const Graph &graph, GraphType type, QGraphicsItem *parent = 0);

	QPainterPath shape() const {return _shape;}
	QRectF boundingRect() const {return _shape.boundingRect();}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	const QRectF &bounds() const {return _bounds;}

	void setScale(qreal sx, qreal sy);
	void setGraphType(GraphType type);
	int id() const {return _id;}
	void setId(int id) {_id = id;}
	void setColor(const QColor &color);
	void setWidth(int width);
	virtual void setUnits(Units units) {Q_UNUSED(units);}

	qreal yAtX(qreal x);
	qreal distanceAtTime(qreal time);

	void redraw();

signals:
	void sliderPositionChanged(qreal);
	void selected(bool);

public slots:
	void emitSliderPositionChanged(qreal);
	void hover(bool hover);

private:
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

	void updatePath();
	void updateShape();
	void updateBounds();

	int _id;
	QPen _pen;
	int _width;

	Graph _graph;
	GraphType _type;

	QPainterPath _path;
	QPainterPath _shape;
	QRectF _bounds;
	qreal _sx, _sy;

	bool _time;
};

#endif // GRAPHITEM_H
