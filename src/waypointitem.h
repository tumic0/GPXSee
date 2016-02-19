#ifndef WAYPOINTITEM_H
#define WAYPOINTITEM_H

#include <QGraphicsItem>
#include "waypoint.h"

class WaypointItem : public QGraphicsItem
{
public:
	WaypointItem(const Waypoint &entry, QGraphicsItem *parent = 0);
	const Waypoint &entry() const {return _entry;}

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

private:
	void updateBoundingRect();

	Waypoint _entry;
	QRectF _boundingRect;
};

#endif // WAYPOINTITEM_H
