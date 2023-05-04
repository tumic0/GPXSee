#include <cmath>
#include <QApplication>
#include <QCursor>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include "map/map.h"
#include "popup.h"
#include "tooltip.h"
#include "areaitem.h"


ToolTip AreaItem::info() const
{
	ToolTip tt;

	if (!_area.name().isEmpty())
		tt.insert(qApp->translate("PolygonItem", "Name"), _area.name());
	if (!_area.description().isEmpty())
		tt.insert(qApp->translate("PolygonItem", "Description"),
		  _area.description());

	return tt;
}

AreaItem::AreaItem(const Area &area, Map *map, GraphicsItem *parent)
  : PlaneItem(parent), _area(area)
{
	_digitalZoom = 0;
	_width = 2;
	_opacity = 0.5;
	_color = Qt::black;
	_penStyle = Qt::SolidLine;

	_pen = QPen(strokeColor(), width());
	_brush = QBrush(fillColor());

	updatePainterPath(map);

	setCursor(Qt::ArrowCursor);
	setAcceptHoverEvents(true);
}

QPainterPath AreaItem::painterPath(Map *map, const Polygon &polygon)
{
	QPainterPath path;

	for (int i = 0; i < polygon.size(); i++) {
		const QVector<Coordinates> &subpath = polygon.at(i);

		path.moveTo(map->ll2xy(subpath.first()));
		for (int j = 1; j < subpath.size(); j++)
			path.lineTo(map->ll2xy(subpath.at(j)));
		path.closeSubpath();
	}

	return path;
}

void AreaItem::updatePainterPath(Map *map)
{
	_painterPath = QPainterPath();

	for (int i = 0; i < _area.polygons().size(); i++)
		_painterPath.addPath(painterPath(map, _area.polygons().at(i)));
}

void AreaItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	painter->setPen(_width ? _pen : QPen(Qt::NoPen));
	painter->drawPath(_painterPath);
	painter->fillPath(_painterPath, _brush);

/*
	QPen p = QPen(QBrush(Qt::red), 0);
	painter->setPen(p);
	painter->drawRect(boundingRect());
*/
}

void AreaItem::setMap(Map *map)
{
	prepareGeometryChange();
	updatePainterPath(map);
}

const QColor &AreaItem::strokeColor() const
{
	return (_useStyle && _area.style().isValid())
	  ? _area.style().stroke() : _color;
}

QColor AreaItem::fillColor() const
{
	if (_useStyle && _area.style().isValid())
		return _area.style().fill();
	else {
		QColor fc(_color);
		fc.setAlphaF(_opacity * _color.alphaF());
		return fc;
	}
}

void AreaItem::setColor(const QColor &color)
{
	_color = color;
	updateColor();
}

void AreaItem::updateColor()
{
	_pen.setColor(strokeColor());
	_brush = QBrush(fillColor());
	update();
}

void AreaItem::setOpacity(qreal opacity)
{
	_opacity = opacity;
	updateColor();
}

qreal AreaItem::width() const
{
	return (_useStyle && _area.style().width() > 0)
	  ? _area.style().width() : _width;
}

void AreaItem::setWidth(qreal width)
{
	_width = width;
	updateWidth();
}

void AreaItem::updateWidth()
{
	prepareGeometryChange();

	_pen.setWidthF(width() * pow(2, -_digitalZoom));
}

Qt::PenStyle AreaItem::penStyle() const
{
	return _useStyle ? Qt::SolidLine : _penStyle;
}

void AreaItem::setPenStyle(Qt::PenStyle style)
{
	_penStyle = style;
	updatePenStyle();
}

void AreaItem::updatePenStyle()
{
	_pen.setStyle(penStyle());
	update();
}

void AreaItem::updateStyle()
{
	updateColor();
	updateWidth();
	updatePenStyle();
}

void AreaItem::setDigitalZoom(int zoom)
{
	if (_digitalZoom == zoom)
		return;

	prepareGeometryChange();

	_digitalZoom = zoom;
	_pen.setWidthF(width() * pow(2, -_digitalZoom));
}

void AreaItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_pen.setWidthF((width() + 1) * pow(2, -_digitalZoom));
	update();
}

void AreaItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_pen.setWidthF(width() * pow(2, -_digitalZoom));
	update();
}
