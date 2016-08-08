#include <QApplication>
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

	if (!_waypoint.name().isEmpty() && !_showLabel)
		tt.insert(qApp->translate("WaypointItem", "Name"), _waypoint.name());
	tt.insert(qApp->translate("WaypointItem", "Coordinates"),
	  ::coordinates(_waypoint.coordinates()));
	if (!std::isnan(_waypoint.elevation()))
		tt.insert(qApp->translate("WaypointItem", "Elevation"),
		  ::elevation(_waypoint.elevation() - _waypoint.geoidHeight(), _units));
	if (!_waypoint.timestamp().isNull())
		tt.insert(qApp->translate("WaypointItem", "Date"),
		  _waypoint.timestamp().toString(Qt::SystemLocaleShortDate));
	if (!_waypoint.description().isNull())
		tt.insert(qApp->translate("WaypointItem", "Description"),
		  _waypoint.description());

	return tt.toString();
}

WaypointItem::WaypointItem(const Waypoint &waypoint, QGraphicsItem *parent)
  : QGraphicsItem(parent)
{
	_units = Metric;
	_showLabel = true;

	_waypoint = waypoint;
	_coordinates = ll2mercator(QPointF(waypoint.coordinates().x(),
	  -waypoint.coordinates().y()));

	updateBoundingRect();

	setPos(_coordinates);
	setToolTip(toolTip());
	setCursor(Qt::ArrowCursor);
}

void WaypointItem::updateBoundingRect()
{
	if (_showLabel) {
		QFont font;
		font.setPixelSize(FONT_SIZE);
		font.setFamily(FONT_FAMILY);
		QFontMetrics fm(font);
		QRect ts = fm.tightBoundingRect(_waypoint.name());

		_boundingRect = QRectF(-POINT_SIZE/2, -POINT_SIZE/2, ts.width()
		  + POINT_SIZE, ts.height() + fm.descent() + POINT_SIZE);
	} else
		_boundingRect = QRectF(-POINT_SIZE/2, -POINT_SIZE/2, POINT_SIZE,
		  POINT_SIZE);
}

void WaypointItem::paint(QPainter *painter,
  const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	if (_showLabel) {
		QFont font;
		font.setPixelSize(FONT_SIZE);
		font.setFamily(FONT_FAMILY);
		QFontMetrics fm(font);
		QRect ts = fm.tightBoundingRect(_waypoint.name());

		painter->setFont(font);
		painter->drawText(POINT_SIZE/2 - qMax(ts.x(), 0), POINT_SIZE/2
		  + ts.height(), _waypoint.name());
	}

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

void WaypointItem::showLabel(bool show)
{
	prepareGeometryChange();
	_showLabel = show;
	updateBoundingRect();
	setToolTip(toolTip());
}
