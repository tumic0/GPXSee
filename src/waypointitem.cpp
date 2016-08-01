#include <QPainter>
#include "config.h"
#include "ll.h"
#include "misc.h"
#include "tooltip.h"
#include "waypointitem.h"


#define POINT_SIZE  8

QString WaypointItem::toolTip()
{
	ToolTip tt;

	tt.insert("Coordinates", ::coordinates(_coordinates));
	if (!std::isnan(_elevation))
		tt.insert("Elevation", ::elevation(_elevation, _units));
	if (!_date.isNull())
		tt.insert("Date", _date.toString(Qt::SystemLocaleShortDate));
	if (!_description.isNull())
		tt.insert("Description", _description);

	return tt.toString();
}

WaypointItem::WaypointItem(const Waypoint &waypoint, QGraphicsItem *parent)
  : QGraphicsItem(parent)
{
	_units = Metric;

	_label = waypoint.name();
	_coordinates = ll2mercator(QPointF(waypoint.coordinates().x(),
	  -waypoint.coordinates().y()));
	_elevation = waypoint.elevation() - waypoint.geoidHeight();
	_description = waypoint.description();
	_date = waypoint.timestamp();

	updateBoundingRect();

	setToolTip(toolTip());
	setCursor(Qt::ArrowCursor);

	setPos(_coordinates);
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

void WaypointItem::setScale(qreal scale)
{
	setPos(_coordinates * scale);
}

void WaypointItem::setUnits(enum Units units)
{
	_units = units;
	setToolTip(toolTip());
}
