#ifndef GRIDITEM_H
#define GRIDITEM_H

#include <QGraphicsItem>

class GridItem : public QGraphicsItem
{
public:
	GridItem(QGraphicsItem *parent = 0);

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void setTicks(const QList<qreal> &x, const QList<qreal> &y);
	void setSize(const QSizeF &size);

private:
	QRectF _boundingRect;
	QList<qreal> _xTicks, _yTicks;
};

#endif // GRIDITEM_H
