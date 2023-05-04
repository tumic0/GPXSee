#ifndef AREAITEM_H
#define AREAITEM_H

#include "data/area.h"
#include "planeitem.h"

class AreaItem : public PlaneItem
{
public:
	AreaItem(const Area &area, Map *map, GraphicsItem *parent = 0);

	QPainterPath shape() const {return _painterPath;}
	QRectF boundingRect() const {return _painterPath.boundingRect();}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	const Area &area() const {return _area;}

	RectC bounds() const {return _area.boundingRect();}
	void setMap(Map *map);

	void setColor(const QColor &color);
	void setOpacity(qreal opacity);
	void setWidth(qreal width);
	void setPenStyle(Qt::PenStyle style);
	void setDigitalZoom(int zoom);
	void updateStyle();

	ToolTip info() const;

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private:
	QPainterPath painterPath(Map *map, const Polygon &polygon);
	void updatePainterPath(Map *map);
	void updateColor();
	void updateWidth();
	void updatePenStyle();
	qreal width() const;
	const QColor &strokeColor() const;
	QColor fillColor() const;
	Qt::PenStyle penStyle() const;

	Area _area;

	qreal _width;
	QColor _color;
	qreal _opacity;
	Qt::PenStyle _penStyle;
	int _digitalZoom;

	QPen _pen;
	QBrush _brush;
	QPainterPath _painterPath;
};

#endif // AREAITEM_H
