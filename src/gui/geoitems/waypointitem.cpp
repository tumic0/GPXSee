#include <QApplication>
#include <QPainter>
#include "config.h"
#include "format.h"
#include "tooltip.h"
#include "waypointitem.h"


#define HS(size) \
	((int)((qreal)size * 1.2))
#define FS(size) \
	((int)((qreal)size * 1.41))

QString WaypointItem::toolTip(Units units)
{
	ToolTip tt;

	if (!_waypoint.name().isEmpty())
		tt.insert(qApp->translate("WaypointItem", "Name"), _waypoint.name());
	tt.insert(qApp->translate("WaypointItem", "Coordinates"),
	  Format::coordinates(_waypoint.coordinates()));
	if (!std::isnan(_waypoint.elevation()))
		tt.insert(qApp->translate("WaypointItem", "Elevation"),
		  Format::elevation(_waypoint.elevation(), units));
	if (!_waypoint.timestamp().isNull())
		tt.insert(qApp->translate("WaypointItem", "Date"),
		  _waypoint.timestamp().toString(Qt::SystemLocaleShortDate));
	if (!_waypoint.description().isNull())
		tt.insert(qApp->translate("WaypointItem", "Description"),
		  _waypoint.description());

	return tt.toString();
}

WaypointItem::WaypointItem(const Waypoint &waypoint, Map *map,
  QGraphicsItem *parent) : QGraphicsItem(parent)
{
	_waypoint = waypoint;
	_showLabel = true;
	_hover = false;
	_size = 8;
	_color = Qt::black;

	updateShape();

	setPos(map->ll2xy(waypoint.coordinates()));
	setToolTip(toolTip(Metric));
	setCursor(Qt::ArrowCursor);
	setAcceptHoverEvents(true);
}

void WaypointItem::updateShape()
{
	QPainterPath p;
	qreal pointSize = _hover ? HS(_size) : _size;

	if (_showLabel) {
		QFont font;
		font.setPixelSize(FS(_size));
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

	qreal pointSize = _hover ? HS(_size) : _size;

	painter->setPen(_color);

	if (_showLabel) {
		QFont font;
		font.setPixelSize(FS(_size));
		font.setFamily(FONT_FAMILY);
		if (_hover)
			font.setBold(true);
		QFontMetrics fm(font);
		QRect ts = fm.tightBoundingRect(_waypoint.name());

		painter->setFont(font);
		painter->drawText(pointSize/2 - qMax(ts.x(), 0), pointSize/2
		  + ts.height(), _waypoint.name());
	}

	painter->setBrush(QBrush(_color, Qt::SolidPattern));
	painter->drawEllipse(-pointSize/2, -pointSize/2, pointSize, pointSize);

/*
	painter->setPen(Qt::red);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(_shape);
*/
}

void WaypointItem::setSize(int size)
{
	prepareGeometryChange();
	_size = size;
	updateShape();
}

void WaypointItem::setColor(const QColor &color)
{
	_color = color;
	update();
}

void WaypointItem::setUnits(enum Units units)
{
	setToolTip(toolTip(units));
}

void WaypointItem::showLabel(bool show)
{
	prepareGeometryChange();
	_showLabel = show;
	updateShape();
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
