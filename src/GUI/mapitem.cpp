#include <QCursor>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include "map/map.h"
#include "popup.h"
#include "tooltip.h"
#include "mapitem.h"


QString MapItem::info() const
{
	ToolTip tt;

	if (!_name.isEmpty())
		tt.insert(tr("Name"), _name);
	if (!_fileName.isEmpty())
		tt.insert(tr("File"), _fileName);

	return tt.toString();
}

MapItem::MapItem(Map *src, Map *map, GraphicsItem *parent)
  : PlaneItem(parent)
{
	_name = src->name();
	_fileName = src->path();
	_bounds = RectC(src->xy2ll(src->bounds().topLeft()),
	  src->xy2ll(src->bounds().bottomRight()));

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

void MapItem::updatePainterPath()
{
	_painterPath = QPainterPath();

	if (_bounds.left() > _bounds.right()) {
		QRectF r1(_map->ll2xy(_bounds.topLeft()), _map->ll2xy(Coordinates(180,
		  _bounds.bottomRight().lat())));
		QRectF r2(_map->ll2xy(Coordinates(-180, _bounds.topLeft().lat())),
		  _map->ll2xy(_bounds.bottomRight()));
		QRectF r(_map->ll2xy(_bounds.topLeft()),
		  _map->ll2xy(_bounds.bottomRight()));

		if (r1.united(r2) == r)
			_painterPath.addRect(r);
		else {
			_painterPath.addRect(r1);
			_painterPath.addRect(r2);
		}
	} else
		_painterPath.addRect(QRectF(_map->ll2xy(_bounds.topLeft()),
		  _map->ll2xy(_bounds.bottomRight())));
}

void MapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	painter->setPen(_width ? _pen : QPen(Qt::NoPen));
	painter->drawPath(_painterPath);
	painter->fillPath(_painterPath, _brush);

	//QPen p = QPen(QBrush(Qt::red), 0);
	//painter->setPen(p);
	//painter->drawRect(boundingRect());
}

void MapItem::setMap(Map *map)
{
	prepareGeometryChange();

	_map = map;

	updatePainterPath();
}

void MapItem::setColor(const QColor &color)
{
	if (_pen.color() == color)
		return;

	QColor bc(color);
	bc.setAlphaF(_opacity * color.alphaF());

	_pen.setColor(color);
	_brush = QBrush(bc);
	update();
}

void MapItem::setOpacity(qreal opacity)
{
	if (_opacity == opacity)
		return;

	_opacity = opacity;
	QColor bc(_pen.color());
	bc.setAlphaF(_opacity * _pen.color().alphaF());
	_brush = QBrush(bc);

	update();
}

void MapItem::setWidth(qreal width)
{
	if (_width == width)
		return;

	prepareGeometryChange();

	_width = width;
	_pen.setWidthF(_width * pow(2, -_digitalZoom));
}

void MapItem::setStyle(Qt::PenStyle style)
{
	if (_pen.style() == style)
		return;

	_pen.setStyle(style);
	update();
}

void MapItem::setDigitalZoom(int zoom)
{
	if (_digitalZoom == zoom)
		return;

	prepareGeometryChange();

	_digitalZoom = zoom;
	_pen.setWidthF(_width * pow(2, -_digitalZoom));
}

void MapItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_pen.setWidthF((_width + 1) * pow(2, -_digitalZoom));
	update();
}

void MapItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_pen.setWidthF(_width * pow(2, -_digitalZoom));
	update();
}

void MapItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
	Q_UNUSED(event);

	emit triggered();
}
