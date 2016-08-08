#ifndef ROUTEITEM_H
#define ROUTEITEM_H

#include <QGraphicsItem>
#include "markeritem.h"
#include "route.h"

class RouteItem : public QGraphicsItem
{
public:
	RouteItem(const Route &route, QGraphicsItem *parent = 0);

	QRectF boundingRect() const {return _path.boundingRect();}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	const QPainterPath &path() const {return _path;}

	void setScale(qreal scale);
	void setColor(const QColor &color);

	void showMarker(bool show) {_marker->setVisible(show);}
	void moveMarker(qreal t);

private:
	QPainterPath _path;
	QPen _pen;

	MarkerItem *_marker;
};

#endif // ROUTEITEM_H
