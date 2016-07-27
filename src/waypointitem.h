#ifndef WAYPOINTITEM_H
#define WAYPOINTITEM_H

#include <QGraphicsItem>
#include "waypoint.h"

class WaypointItem : public QGraphicsItem
{
public:
	WaypointItem(const Waypoint &waypoint, QGraphicsItem *parent = 0);
	const QPointF &coordinates() {return _coordinates;}

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

private:
	void updateBoundingRect();

	QString _label;
	QPointF _coordinates;
	QRectF _boundingRect;
};

#endif // WAYPOINTITEM_H
