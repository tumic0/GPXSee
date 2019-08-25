#ifndef GRAPHITEM_H
#define GRAPHITEM_H

#include <QGraphicsObject>
#include <QPen>
#include "data/graph.h"
#include "units.h"

class GraphItem : public QGraphicsObject
{
	Q_OBJECT

public:
	GraphItem(const Graph &graph, GraphType type, int width, const QColor &color,
	  QGraphicsItem *parent = 0);
	virtual ~GraphItem() {}

	QPainterPath shape() const {return _shape;}
	QRectF boundingRect() const {return _shape.boundingRect();}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	const QRectF &bounds() const {return _bounds;}

	qreal max() const;
	qreal min() const;
	qreal avg() const;

	void setScale(qreal sx, qreal sy);
	void setGraphType(GraphType type);
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

	const GraphSegment *segment(qreal x, GraphType type) const;
	void updatePath();
	void updateShape();
	void updateBounds();

	Graph _graph;
	GraphType _type;
	QPainterPath _path;
	QPainterPath _shape;
	QRectF _bounds;
	qreal _sx, _sy;
	QPen _pen;

	bool _time;
};

#endif // GRAPHITEM_H
