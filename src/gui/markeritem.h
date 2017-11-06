#ifndef MARKERITEM_H
#define MARKERITEM_H

#include <QGraphicsItem>

class MarkerItem : public QGraphicsItem
{
public:
	MarkerItem(QGraphicsItem *parent = 0);

	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);
};

#endif // MARKERITEM_H
