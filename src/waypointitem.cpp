#include <QPainter>
#include "config.h"
#include "waypointitem.h"


#define POINT_SIZE  8

WaypointItem::WaypointItem(const Waypoint &entry, QGraphicsItem *parent)
  : QGraphicsItem(parent)
{
	_entry = entry;
	updateBoundingRect();

	if (!entry.description().isEmpty()) {
		setToolTip(entry.description());
		setCursor(Qt::ArrowCursor);
	}
}

void WaypointItem::updateBoundingRect()
{
	QFont font;
	font.setPixelSize(FONT_SIZE);
	font.setFamily(FONT_FAMILY);
	QFontMetrics fm(font);
	QRect ts = fm.tightBoundingRect(_entry.name());

	_boundingRect = QRectF(-POINT_SIZE/2, -POINT_SIZE/2, ts.width()
	  + POINT_SIZE, ts.height() + fm.descent() + POINT_SIZE);
}

void WaypointItem::paint(QPainter *painter,
  const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	QFont font;
	font.setPixelSize(FONT_SIZE);
	font.setFamily(FONT_FAMILY);
	QFontMetrics fm(font);
	QRect ts = fm.tightBoundingRect(_entry.name());

	painter->setFont(font);
	painter->drawText(POINT_SIZE/2 - qMax(ts.x(), 0), POINT_SIZE/2 + ts.height(),
	  _entry.name());
	painter->setBrush(Qt::SolidPattern);
	painter->drawEllipse(-POINT_SIZE/2, -POINT_SIZE/2, POINT_SIZE, POINT_SIZE);

/*
	painter->setPen(Qt::red);
	painter->setBrush(Qt::NoBrush);
	painter->drawRect(boundingRect());
*/
}
