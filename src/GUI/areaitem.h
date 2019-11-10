#ifndef AREAITEM_H
#define AREAITEM_H

#include "data/area.h"
#include "graphicsscene.h"
#include "tooltip.h"

class Map;

class AreaItem : public GraphicsItem
{
public:
	AreaItem(const Area &area, Map *map, GraphicsItem *parent = 0);

	QPainterPath shape() const {return _painterPath;}
	QRectF boundingRect() const {return _painterPath.boundingRect();}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	const Area &area() const {return _area;}

	void setMap(Map *map);

	void setColor(const QColor &color);
	void setOpacity(qreal opacity);
	void setWidth(qreal width);
	void setStyle(Qt::PenStyle style);
	void setDigitalZoom(int zoom);

	virtual QString info() const;

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
	QPainterPath painterPath(const Polygon &polygon);
	void updatePainterPath();
	ToolTip toolTip() const;

	Area _area;
	Map *_map;
	int _digitalZoom;

	qreal _width;
	QPen _pen;
	QBrush _brush;
	qreal _opacity;

	QPainterPath _painterPath;
};

#endif // AREAITEM_H
