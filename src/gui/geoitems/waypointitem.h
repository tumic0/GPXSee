#ifndef WAYPOINTITEM_H
#define WAYPOINTITEM_H

#include <cmath>
#include <QGraphicsItem>
#include "waypoint.h"
#include "units.h"
#include "map.h"

class WaypointItem : public QGraphicsItem
{
public:
	WaypointItem(const Waypoint &waypoint, Map *map, QGraphicsItem *parent = 0);

	const Waypoint &waypoint() const {return _waypoint;}

	void setMap(Map *map) {setPos(map->ll2xy(_waypoint.coordinates()));}
	void setUnits(Units units);
	void setSize(int size);
	void setColor(const QColor &color);
	void showLabel(bool show);
	void setDigitalZoom(int zoom) {setScale(pow(2, -zoom));}

	QPainterPath shape() const {return _shape;}
	QRectF boundingRect() const {return _shape.boundingRect();}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

private:
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

	void updateShape();
	QString toolTip(Units units);

	QPainterPath _shape;
	Waypoint _waypoint;

	QColor _color;
	int _size;
	bool _hover;
	bool _showLabel;
};

#endif // WAYPOINTITEM_H
