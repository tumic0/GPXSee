#ifndef AXISITEM_H
#define AXISITEM_H

#include <QGraphicsItem>
#include "common/range.h"

class AxisItem : public QGraphicsItem
{
public:
	enum Type {X, Y};

	AxisItem(Type type, QGraphicsItem *parent = 0);

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void setRange(const RangeF &range);
	void setSize(qreal size);
	void setLabel(const QString& label);

	QSizeF margin() const;
	QList<qreal> ticks() const;

private:
	void updateBoundingRect();

	Type _type;
	RangeF _range;
	qreal _size;
	QString _label;
	QRectF _boundingRect;
};

#endif // AXISITEM_H
