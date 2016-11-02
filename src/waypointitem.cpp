#include <QApplication>
#include <QPainter>
#include "config.h"
#include "format.h"
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
	  Format::coordinates(_waypoint.coordinates()));
	if (!std::isnan(_waypoint.elevation()))
		tt.insert(qApp->translate("WaypointItem", "Elevation"),
		  Format::elevation(_waypoint.elevation(), _units));
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
	QPointF p = waypoint.coordinates().toMercator();
	_coordinates = QPointF(p.x(), -p.y());

	updateShape();

	setPos(_coordinates);
	setToolTip(toolTip());
	setCursor(Qt::ArrowCursor);
	setAcceptHoverEvents(true);
}

void WaypointItem::updateShape()
{
	QPainterPath p;
	qreal pointSize = _hover ? HOVER_SIZE : POINT_SIZE;

	if (_showLabel) {
		QFont font;
		font.setPixelSize(FONT_SIZE);
		font.setFamily(FONT_FAMILY);
		if (_hover)
			font.setBold(true);
		QFontMetrics fm(font);
		QRect ts = fm.tightBoundingRect(_waypoint.name());

		p.addRect(-pointSize/2, -pointSize/2, pointSize, pointSize);
		p.addRect(pointSize/2, pointSize/2,
		  ts.width(), ts.height() + fm.descent());
	} else
		p.addRect(-pointSize/2, -pointSize/2, pointSize, pointSize);

	_shape = p;
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
	painter->drawPath(_shape);
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
	updateShape();
	setToolTip(toolTip());
}

void WaypointItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	prepareGeometryChange();
	_hover = true;
	updateShape();
	setZValue(zValue() + 1.0);
}

void WaypointItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	prepareGeometryChange();
	_hover = false;
	updateShape();
	setZValue(zValue() - 1.0);
}
