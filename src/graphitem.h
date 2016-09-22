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
	  {return (_type == Distance) ? _distancePath.boundingRect()
	  : _timePath.boundingRect();}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void setGraphType(GraphType type) {_type = type;}
	int id() const {return _id;}
	void setId(int id) {_id = id;}
	void setColor(const QColor &color);

	qreal yAtX(qreal x);
	qreal distanceAtTime(qreal time);

signals:
	void sliderPositionChanged(qreal);

public slots:
	void emitSliderPositionChanged(qreal);
	void selected(bool selected);

private:
	int _id;
	QPen _pen;
	QPainterPath _distancePath, _timePath;
	GraphType _type;
};

#endif // GRAPHITEM_H
