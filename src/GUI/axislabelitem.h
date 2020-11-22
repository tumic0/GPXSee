#ifndef AXISLABELITEM_H
#define AXISLABELITEM_H

#include <QGraphicsItem>
#include <QFont>

class AxisLabelItem : public QGraphicsItem
{
public:
	enum Type {X, Y};

	AxisLabelItem(Type type, QGraphicsItem *parent = 0);

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void setLabel(const QString& label, const QString &units);

private:
	void updateBoundingRect();

	Type _type;
	QString _label;
	QFont _font;
	QRect _labelBB;
	QRectF _boundingRect;
};

#endif // AXISLABELITEM_H
