#include <QApplication>
#include <QPainter>
#include "font.h"
#include "tooltip.h"
#include "waypointitem.h"


#define HS(size) \
	((int)((qreal)size * 1.2))
#define FS(size) \
	((int)((qreal)size * 1.41))

QString WaypointItem::toolTip(Units units, CoordinatesFormat format)
{
	ToolTip tt;

	if (!_waypoint.name().isEmpty())
		tt.insert(qApp->translate("WaypointItem", "Name"), _waypoint.name());
	tt.insert(qApp->translate("WaypointItem", "Coordinates"),
	  Format::coordinates(_waypoint.coordinates(), format));
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
	_size = 8;
	_color = Qt::black;

	_font.setPixelSize(FS(_size));
	_font.setFamily(FONT_FAMILY);

	updateCache();

	setPos(map->ll2xy(waypoint.coordinates()));
	setToolTip(toolTip(Metric, DecimalDegrees));
	setCursor(Qt::ArrowCursor);
	setAcceptHoverEvents(true);
}

void WaypointItem::updateCache()
{
	QPainterPath p;
	qreal pointSize = _font.bold() ? HS(_size) : _size;

	if (_showLabel) {
		QFontMetrics fm(_font);
		_labelBB = fm.tightBoundingRect(_waypoint.name());

		p.addRect(-pointSize/2, -pointSize/2, pointSize, pointSize);
		p.addRect(pointSize/2, pointSize/2, _labelBB.width(), _labelBB.height()
		  + fm.descent());
	} else
		p.addRect(-pointSize/2, -pointSize/2, pointSize, pointSize);

	_shape = p;
}

void WaypointItem::paint(QPainter *painter,
  const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	qreal pointSize = _font.bold() ? HS(_size) : _size;

	painter->setPen(_color);

	if (_showLabel) {
		painter->setFont(_font);
		painter->drawText(pointSize/2 - qMax(_labelBB.x(), 0), pointSize/2
		  + _labelBB.height(), _waypoint.name());
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
	if (_size == size)
		return;

	prepareGeometryChange();
	_size = size;
	_font.setPixelSize(FS(_size));
	updateCache();
}

void WaypointItem::setColor(const QColor &color)
{
	if (_color == color)
		return;

	_color = color;
	update();
}

void WaypointItem::setToolTipFormat(Units units, CoordinatesFormat format)
{
	setToolTip(toolTip(units, format));
}

void WaypointItem::showLabel(bool show)
{
	if (_showLabel == show)
		return;

	prepareGeometryChange();
	_showLabel = show;
	updateCache();
}

void WaypointItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	prepareGeometryChange();
	_font.setBold(true);
	updateCache();
	setZValue(zValue() + 1.0);
}

void WaypointItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	prepareGeometryChange();
	_font.setBold(false);
	updateCache();
	setZValue(zValue() - 1.0);
}
