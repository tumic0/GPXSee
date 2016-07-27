#include <QPainter>
#include "config.h"
#include "ll.h"
#include "misc.h"
#include "waypointitem.h"


#define POINT_SIZE  8

static QString tt(const Waypoint &waypoint)
{
	QString date = waypoint.timestamp().toString(Qt::SystemLocaleShortDate);

	return "<b>" + QObject::tr("Coordinates:") + "</b> "
	  + coordinates(waypoint.coordinates()) + "<br><b>"
	  + QObject::tr("Description:") + "</b> " + waypoint.description()
	  + "<br><b>" + QObject::tr("Elevation:") + "</b> "
	  + QString::number(waypoint.elevation() - waypoint.geoidHeight())
	  + "<br><b>" + QObject::tr("Date:") + "</b> " + date;
}

WaypointItem::WaypointItem(const Waypoint &waypoint, QGraphicsItem *parent)
  : QGraphicsItem(parent)
{
	_label = waypoint.name();
	_coordinates = ll2mercator(QPointF(waypoint.coordinates().x(),
	  -waypoint.coordinates().y()));

	updateBoundingRect();

	setToolTip(tt(waypoint));
	setCursor(Qt::ArrowCursor);
}

void WaypointItem::updateBoundingRect()
{
	QFont font;
	font.setPixelSize(FONT_SIZE);
	font.setFamily(FONT_FAMILY);
	QFontMetrics fm(font);
	QRect ts = fm.tightBoundingRect(_label);

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
	QRect ts = fm.tightBoundingRect(_label);

	painter->setFont(font);
	painter->drawText(POINT_SIZE/2 - qMax(ts.x(), 0), POINT_SIZE/2 + ts.height(),
	  _label);
	painter->setBrush(Qt::SolidPattern);
	painter->drawEllipse(-POINT_SIZE/2, -POINT_SIZE/2, POINT_SIZE, POINT_SIZE);

/*
	painter->setPen(Qt::red);
	painter->setBrush(Qt::NoBrush);
	painter->drawRect(boundingRect());
*/
}
