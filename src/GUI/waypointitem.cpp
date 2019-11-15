#include <QApplication>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QLabel>
#include "font.h"
#include "tooltip.h"
#include "popup.h"
#include "waypointitem.h"


#define HS(size) \
	((int)((qreal)size * 1.2))
#define FS(size) \
	((int)((qreal)size * 1.41))

QString WaypointItem::info() const
{
	ToolTip tt;

	if (!_waypoint.name().isEmpty())
		tt.insert(qApp->translate("WaypointItem", "Name"), _waypoint.name());
	tt.insert(qApp->translate("WaypointItem", "Coordinates"),
	  Format::coordinates(_waypoint.coordinates(), _format));
	if (_waypoint.hasElevation())
		tt.insert(qApp->translate("WaypointItem", "Elevation"),
		  Format::elevation(_waypoint.elevation(), _units));
	if (_waypoint.timestamp().isValid())
		tt.insert(qApp->translate("WaypointItem", "Date"),
		  _waypoint.timestamp().toString(Qt::SystemLocaleShortDate));
	if (!_waypoint.description().isEmpty())
		tt.insert(qApp->translate("WaypointItem", "Description"),
		  _waypoint.description());
	if (_waypoint.address().isValid()) {
		QString addr("<address>");
		addr += _waypoint.address().street();
		addr += "<br/>" + _waypoint.address().city();
		if (!_waypoint.address().postalCode().isEmpty())
			addr += "<br/>" + _waypoint.address().postalCode();
		if (!_waypoint.address().state().isEmpty())
			addr += "<br/>" + _waypoint.address().state();
		if (!_waypoint.address().country().isEmpty())
			addr += "<br/>" + _waypoint.address().country();
		addr += "</address>";
		tt.insert(qApp->translate("WaypointItem", "Address"), addr);
	}
	if (!_waypoint.links().isEmpty()) {
		QString links;
		for (int i = 0; i < _waypoint.links().size(); i++) {
			const Link &link = _waypoint.links().at(i);
			links.append(QString("<a href=\"%0\">%1</a>").arg(link.URL(),
			  link.text().isEmpty() ? link.URL() : link.text()));
			if (i != _waypoint.links().size() - 1)
				links.append("<br/>");
		}
		tt.insert(qApp->translate("WaypointItem", "Links"), links);
	}
	tt.setImages(_waypoint.images());

	return tt.toString();
}

WaypointItem::WaypointItem(const Waypoint &waypoint, Map *map,
  QGraphicsItem *parent) : GraphicsItem(parent)
{
	_waypoint = waypoint;
	_showLabel = true;
	_size = 8;
	_color = Qt::black;

	_font.setPixelSize(FS(_size));
	_font.setFamily(FONT_FAMILY);

	_units = Metric;
	_format = DecimalDegrees;

	updateCache();

	setPos(map->ll2xy(waypoint.coordinates()));
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
	_units = units;
	_format = format;
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

void WaypointItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	Popup::show(event->screenPos(), info(), event->widget());
	/* Do not propagate the event any further as lower stacked items (path
	   items) would replace the popup with their own popup */
}
