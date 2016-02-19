#include <QPainter>
#include "config.h"
#include "waypointitem.h"


#define POINT_SIZE  8

WaypointItem::WaypointItem(const Waypoint &entry, QGraphicsItem *parent)
  : QGraphicsItem(parent)
{
	_entry = entry;
	updateBoundingRect();
}

void WaypointItem::updateBoundingRect()
{
	QFont font;
	font.setPixelSize(FONT_SIZE);
	font.setFamily(FONT_FAMILY);
	QFontMetrics fm(font);
	QRect ts = fm.tightBoundingRect(_entry.description());

	_boundingRect = QRectF(0, 0, ts.width() + POINT_SIZE,
	  ts.height() + fm.descent() + POINT_SIZE);
}

void WaypointItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	QFont font;
	font.setPixelSize(FONT_SIZE);
	font.setFamily(FONT_FAMILY);
	QFontMetrics fm(font);
	QRect ts = fm.tightBoundingRect(_entry.description());

	painter->setFont(font);
	painter->drawText(POINT_SIZE - qMax(ts.x(), 0), POINT_SIZE + ts.height(),
	  _entry.description());
	painter->setBrush(Qt::SolidPattern);
	painter->drawEllipse(0, 0, POINT_SIZE, POINT_SIZE);

/*
	painter->setPen(Qt::red);
	painter->setBrush(Qt::NoBrush);
	painter->drawRect(boundingRect());
*/
}
