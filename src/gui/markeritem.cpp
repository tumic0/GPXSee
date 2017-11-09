#include <QPainter>
#include "markeritem.h"


#define SIZE  8
#define WIDTH 2

MarkerItem::MarkerItem(QGraphicsItem *parent) : QGraphicsItem(parent)
{

}

QRectF MarkerItem::boundingRect() const
{
	return QRectF(-SIZE/2, -SIZE/2, SIZE, SIZE);
}

void MarkerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->setPen(QPen(Qt::red, WIDTH));
	painter->drawLine(-SIZE/2, 0, SIZE/2, 0);
	painter->drawLine(0, -SIZE/2, 0, SIZE/2);

//	painter->drawRect(boundingRect());
}
