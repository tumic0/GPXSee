#include <QBrush>
#include <QPen>
#include "graphitem.h"

void GraphItem::setColor(const QColor &color)
{
	QBrush brush(color, Qt::SolidPattern);
	QPen pen(brush, 0);
	setPen(pen);
}
