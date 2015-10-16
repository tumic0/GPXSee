#ifndef AXISITEM_H
#define AXISITEM_H

#include <QGraphicsItem>

class AxisItem : public QGraphicsItem
{
public:
	enum Type {X, Y};

	AxisItem(Type type, QGraphicsItem *parent = 0);

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void setRange(const QPointF &range);
	void setSize(qreal size);
	void setLabel(const QString& label);

	QSizeF margin();

private:
	void updateBoundingRect();

	Type _type;
	QPointF _range;
	qreal _size;
	QString _label;
	QRectF _boundingRect;
};

#endif // AXISITEM_H
