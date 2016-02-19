#ifndef WAYPOINTITEM_H
#define WAYPOINTITEM_H

#include <QGraphicsItem>
#include "waypoint.h"

class WayPointItem : public QGraphicsItem
{
public:
	WayPointItem(const WayPoint &entry, QGraphicsItem *parent = 0);
	const WayPoint &entry() const {return _entry;}

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

private:
	void updateBoundingRect();

	WayPoint _entry;
	QRectF _boundingRect;
};

#endif // WAYPOINTITEM_H
