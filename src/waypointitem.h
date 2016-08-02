#ifndef WAYPOINTITEM_H
#define WAYPOINTITEM_H

#include <QGraphicsItem>
#include "waypoint.h"
#include "units.h"

class WaypointItem : public QGraphicsItem
{
public:
	WaypointItem(const Waypoint &waypoint, QGraphicsItem *parent = 0);

	const Waypoint &waypoint() const {return _waypoint;}
	const QPointF &coordinates() const {return _coordinates;}

	void setUnits(enum Units units);
	void setScale(qreal scale);

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

private:
	void updateBoundingRect();
	QString toolTip();

	QRectF _boundingRect;
	QPointF _coordinates;
	Waypoint _waypoint;
	Units _units;
};

#endif // WAYPOINTITEM_H
