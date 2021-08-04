#include <cmath>
#include <QCursor>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include "map/map.h"
#include "mapaction.h"
#include "popup.h"
#include "tooltip.h"
#include "mapitem.h"


static void growLeft(Map *map, const Coordinates &c, QRectF &rect)
{
	QPointF p(map->ll2xy(c));

	if (p.x() < rect.left())
		rect.setLeft(p.x());
}

static void growRight(Map *map, const Coordinates &c, QRectF &rect)
{

	QPointF p(map->ll2xy(c));

	if (p.x() > rect.right())
		rect.setRight(p.x());
}

static void growTop(Map *map, const Coordinates &c, QRectF &rect)
{
	QPointF p(map->ll2xy(c));

	if (p.y() > rect.top())
		rect.setTop(p.y());
}

static void growBottom(Map *map, const Coordinates &c, QRectF &rect)
{
	QPointF p(map->ll2xy(c));

	if (p.y() < rect.bottom())
		rect.setBottom(p.y());
}

static QRectF bbox(const RectC &rect, Map *map, int samples = 100)
{
	if (!rect.isValid())
		return QRectF();

	double dx = rect.width() / samples;
	double dy = rect.height() / samples;

	QPointF tl(map->ll2xy(rect.topLeft()));
	QPointF br(map->ll2xy(rect.bottomRight()));
	QRectF prect(tl, br);

	for (int i = 0; i <= samples; i++) {
		double x = remainder(rect.left() + i * dx, 360.0);
		growTop(map, Coordinates(x, rect.bottom()), prect);
		growBottom(map, Coordinates(x, rect.top()), prect);
	}

	for (int i = 0; i <= samples; i++) {
		double y = rect.bottom() + i * dy;
		growLeft(map, Coordinates(rect.left(), y), prect);
		growRight(map, Coordinates(rect.right(), y), prect);
	}

	return prect;
}

ToolTip MapItem::info() const
{
	ToolTip tt;

	if (!_name.isEmpty())
		tt.insert(tr("Name"), _name);
	if (!_fileName.isEmpty())
		tt.insert(tr("File"), _fileName);

	return tt;
}

MapItem::MapItem(MapAction *action, Map *map, GraphicsItem *parent)
  : PlaneItem(parent)
{
	Map *src = action->data().value<Map*>();
	Q_ASSERT(map->isReady());

	_name = src->name();
	_fileName = src->path();
	_bounds = src->llBounds();

	connect(this, &MapItem::triggered, action, &MapAction::trigger);

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

	QRectF r(bbox(_bounds, _map));

	if (r.left() > r.right()) {
		QRectF r1(bbox(RectC(_bounds.topLeft(),
		  Coordinates(180, _bounds.bottomRight().lat())), _map));
		QRectF r2(bbox(RectC(Coordinates(-180, _bounds.topLeft().lat()),
		  _bounds.bottomRight()), _map));

		_painterPath.addRect(r1);
		_painterPath.addRect(r2);
	} else
		_painterPath.addRect(r);
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
