#include <QApplication>
#include <QPainter>
#include "config.h"
#include "ll.h"
#include "misc.h"
#include "tooltip.h"
#include "waypointitem.h"


#define POINT_SIZE 8
#define HOVER_SIZE 10

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
	_hover = false;

	_waypoint = waypoint;
	_coordinates = ll2mercator(QPointF(waypoint.coordinates().x(),
	  -waypoint.coordinates().y()));

	updateBoundingRect();

	setPos(_coordinates);
	setToolTip(toolTip());
	setCursor(Qt::ArrowCursor);
	setAcceptHoverEvents(true);
}

void WaypointItem::updateBoundingRect()
{
	qreal pointSize = _hover ? HOVER_SIZE : POINT_SIZE;

	if (_showLabel) {
		QFont font;
		font.setPixelSize(FONT_SIZE);
		font.setFamily(FONT_FAMILY);
		if (_hover)
			font.setBold(true);
		QFontMetrics fm(font);
		QRect ts = fm.tightBoundingRect(_waypoint.name());

		_boundingRect = QRectF(-pointSize/2, -pointSize/2, ts.width()
		  + pointSize, ts.height() + fm.descent() + pointSize);
	} else
		_boundingRect = QRectF(-pointSize/2, -pointSize/2, pointSize,
		  pointSize);
}

void WaypointItem::paint(QPainter *painter,
  const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	qreal pointSize = _hover ? HOVER_SIZE : POINT_SIZE;

	if (_showLabel) {
		QFont font;
		font.setPixelSize(FONT_SIZE);
		font.setFamily(FONT_FAMILY);
		if (_hover)
			font.setBold(true);
		QFontMetrics fm(font);
		QRect ts = fm.tightBoundingRect(_waypoint.name());

		painter->setFont(font);
		painter->drawText(pointSize/2 - qMax(ts.x(), 0), pointSize/2
		  + ts.height(), _waypoint.name());
	}

	painter->setBrush(Qt::SolidPattern);
	painter->drawEllipse(-pointSize/2, -pointSize/2, pointSize, pointSize);

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

void WaypointItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	prepareGeometryChange();
	_hover = true;
	updateBoundingRect();
	setZValue(1.0);
}

void WaypointItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	prepareGeometryChange();
	_hover = false;
	updateBoundingRect();
	setZValue(0);
}
