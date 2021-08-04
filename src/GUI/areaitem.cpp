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
	_map = map;
	_digitalZoom = 0;

	_width = 2;
	_opacity = 0.5;
	QBrush brush(Qt::SolidPattern);
	_pen = QPen(brush, _width);

	updatePainterPath();

	setCursor(Qt::ArrowCursor);
	setAcceptHoverEvents(true);
}

QPainterPath AreaItem::painterPath(const Polygon &polygon)
{
	QPainterPath path;

	for (int i = 0; i < polygon.size(); i++) {
		const QVector<Coordinates> &subpath = polygon.at(i);

		path.moveTo(_map->ll2xy(subpath.first()));
		for (int j = 1; j < subpath.size(); j++)
			path.lineTo(_map->ll2xy(subpath.at(j)));
		path.closeSubpath();
	}

	return path;
}

void AreaItem::updatePainterPath()
{
	_painterPath = QPainterPath();

	for (int i = 0; i < _area.polygons().size(); i++)
		_painterPath.addPath(painterPath(_area.polygons().at(i)));
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

	_map = map;

	updatePainterPath();
}

void AreaItem::setColor(const QColor &color)
{
	if (_pen.color() == color)
		return;

	QColor bc(color);
	bc.setAlphaF(_opacity * color.alphaF());

	_pen.setColor(color);
	_brush = QBrush(bc);
	update();
}

void AreaItem::setOpacity(qreal opacity)
{
	if (_opacity == opacity)
		return;

	_opacity = opacity;
	QColor bc(_pen.color());
	bc.setAlphaF(_opacity * _pen.color().alphaF());
	_brush = QBrush(bc);

	update();
}

void AreaItem::setWidth(qreal width)
{
	if (_width == width)
		return;

	prepareGeometryChange();

	_width = width;
	_pen.setWidthF(_width * pow(2, -_digitalZoom));
}

void AreaItem::setStyle(Qt::PenStyle style)
{
	if (_pen.style() == style)
		return;

	_pen.setStyle(style);
	update();
}

void AreaItem::setDigitalZoom(int zoom)
{
	if (_digitalZoom == zoom)
		return;

	prepareGeometryChange();

	_digitalZoom = zoom;
	_pen.setWidthF(_width * pow(2, -_digitalZoom));
}

void AreaItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_pen.setWidthF((_width + 1) * pow(2, -_digitalZoom));
	update();
}

void AreaItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_pen.setWidthF(_width * pow(2, -_digitalZoom));
	update();
}
