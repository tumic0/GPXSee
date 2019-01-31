#ifndef AREAITEM_H
#define AREAITEM_H

#include <QGraphicsItem>
#include "data/area.h"

class Map;

class AreaItem : public QGraphicsItem
{
public:
	AreaItem(const Area &area, Map *map, QGraphicsItem *parent = 0);

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

private:
	QPainterPath painterPath(const Polygon &polygon);
	void updatePainterPath();
	QString toolTip() const;

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
