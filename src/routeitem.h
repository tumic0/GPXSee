#ifndef ROUTEITEM_H
#define ROUTEITEM_H

#include <QPen>
#include "pathitem.h"
#include "markeritem.h"
#include "route.h"
#include "units.h"


class RouteItem : public PathItem
{
	Q_OBJECT

public:
	RouteItem(const Route &route, QGraphicsItem *parent = 0);

	QPainterPath shape() const {return _shape;}
	QRectF boundingRect() const {return _shape.boundingRect();}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	const QPainterPath &path() const {return _path;}

	void setScale(qreal scale);
	void setColor(const QColor &color);
	void setUnits(enum Units units);

	void showMarker(bool show) {_marker->setVisible(show);}
	void moveMarker(qreal distance);

	void showWaypoints(bool show);
	void showWaypointLabels(bool show);

private:
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

	void updateShape();
	QString toolTip();

	QPainterPath _path;
	QPainterPath _shape;
	QPen _pen;

	MarkerItem *_marker;

	Units _units;
	qreal _distance;
};

#endif // ROUTEITEM_H
