#ifndef MARKERINFOITEM_H
#define MARKERINFOITEM_H

#include <QGraphicsItem>

class MarkerInfoItem : public QGraphicsItem
{
public:
	MarkerInfoItem(QGraphicsItem *parent = 0);

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void setDate(const QDateTime &date);
	void setColor(const QColor &color);

private:
	void updateBoundingRect();

	QString _date, _time;
	QRectF _boundingRect;
	QColor _color;
	QFont _font;
};

#endif // MARKERINFOITEM_H
