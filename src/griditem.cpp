#include <QPainter>
#include "griditem.h"


#define GRID_WIDTH 0

GridItem::GridItem(QGraphicsItem *parent) : QGraphicsItem(parent)
{
#ifndef Q_OS_MAC
	setCacheMode(QGraphicsItem::DeviceCoordinateCache);
#endif // Q_OS_MAC
}

void GridItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	QBrush brush(Qt::gray);
	QPen pen = QPen(brush, GRID_WIDTH, Qt::DotLine);


	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->setPen(pen);

	for (int i = 0; i < _xTicks.size(); i++)
		painter->drawLine(_xTicks.at(i), 0, _xTicks.at(i),
		  -_boundingRect.height());
	for (int i = 0; i < _yTicks.size(); i++)
		painter->drawLine(0, -_yTicks.at(i), boundingRect().width(),
		  -_yTicks.at(i));

/*
	painter->setPen(Qt::red);
	painter->drawRect(boundingRect());
*/
}

void GridItem::setTicks(const QList<qreal> &x, const QList<qreal> &y)
{
	_xTicks = x; _yTicks = y;
	update();
}

void GridItem::setSize(const QSizeF &size)
{
	prepareGeometryChange();

	_boundingRect = QRectF(QPointF(0, -size.height()), size);
}
