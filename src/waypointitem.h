#ifndef WAYPOINTITEM_H
#define WAYPOINTITEM_H

#include <QGraphicsItem>
#include "waypoint.h"
#include "units.h"

class WaypointItem : public QGraphicsItem
{
public:
	WaypointItem(const Waypoint &waypoint, QGraphicsItem *parent = 0);
	const QPointF &coordinates() {return _coordinates;}

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void setUnits(enum Units units);
	void setScale(qreal scale);

private:
	void updateBoundingRect();
	QString toolTip();

	QString _label;
	QPointF _coordinates;
	QRectF _boundingRect;

	Units _units;
	QDateTime _date;
	QString _description;
	qreal _elevation;
};

#endif // WAYPOINTITEM_H
