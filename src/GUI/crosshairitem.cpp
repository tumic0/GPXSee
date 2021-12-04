#include <QPainter>
#include "crosshairitem.h"

#define SIZE  16
#define WIDTH 2

CrosshairItem::CrosshairItem(QGraphicsItem *parent) : QGraphicsItem(parent)
{
	_color = Qt::red;
	_digitalZoom = 0;
}

QRectF CrosshairItem::boundingRect() const
{
	return QRectF(-SIZE/2, -SIZE/2, SIZE, SIZE);
}

void CrosshairItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->setPen(QPen(_color, WIDTH));

	painter->drawLine(-SIZE/2, 0, -SIZE/4, 0);
	painter->drawLine(SIZE/4, 0, SIZE/2, 0);
	painter->drawLine(0, -SIZE/2, 0, -SIZE/4);
	painter->drawLine(0, SIZE/4, 0, SIZE/2);

	//painter->drawRect(boundingRect());
}

void CrosshairItem::setDigitalZoom(qreal zoom)
{
	_digitalZoom = zoom;
	setScale(pow(2, -_digitalZoom));
}

void CrosshairItem::setColor(const QColor &color)
{
	_color = color;
	update();
}
