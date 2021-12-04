#ifndef CROSSHAIRITEM_H
#define CROSSHAIRITEM_H

#include <QGraphicsItem>
#include "map/map.h"

class CrosshairItem : public QGraphicsItem
{
public:
	CrosshairItem(QGraphicsItem *parent = 0);

	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	const Coordinates &coordinates() const {return _c;}
	const QColor &color() const {return _color;}

	void setCoordinates(const Coordinates &c) {_c = c;}
	void setMap(Map *map) {setPos(map->ll2xy(_c));}
	void setDigitalZoom(qreal zoom);
	void setColor(const QColor &color);

private:
	Coordinates _c;
	QColor _color;
	qreal _digitalZoom;
};

#endif // CROSSHAIRITEM_H
