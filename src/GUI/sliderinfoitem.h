#ifndef SLIDERINFOITEM_H
#define SLIDERINFOITEM_H

#include <QGraphicsItem>

class SliderInfoItem : public QGraphicsItem
{
public:
	enum Side {Left, Right};

	SliderInfoItem(QGraphicsItem *parent = 0);

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void setText(const QString &x, const QString &y);
	void setSide(Side side);
	void setColor(const QColor &color);

private:
	void updateBoundingRect();

	Side _side;
	QString _x, _y;
	QRectF _boundingRect;
	QColor _color;
	QFont _font;
};

#endif // SLIDERINFOITEM_H
