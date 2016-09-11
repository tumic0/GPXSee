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
	void showLabel(bool show);

	QPainterPath shape() const {return _shape;}
	QRectF boundingRect() const {return _shape.boundingRect();}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

private:
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

	void updateShape();
	QString toolTip();

	QPainterPath _shape;
	QPointF _coordinates;
	Waypoint _waypoint;
	Units _units;

	bool _hover;
	bool _showLabel;
};

#endif // WAYPOINTITEM_H
