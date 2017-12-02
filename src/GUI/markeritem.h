#ifndef MARKERITEM_H
#define MARKERITEM_H

#include <QGraphicsItem>
#include <QColor>

class MarkerItem : public QGraphicsItem
{
public:
	MarkerItem(QGraphicsItem *parent = 0);

	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void setColor(const QColor &color);

private:
	QColor _color;
};

#endif // MARKERITEM_H
