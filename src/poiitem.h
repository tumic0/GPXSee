#ifndef POIITEM_H
#define POIITEM_H

#include <QGraphicsItem>
#include "poi.h"

class POIItem : public QGraphicsItem
{
public:
	POIItem(const WayPoint &entry, QGraphicsItem *parent = 0);
	const WayPoint &entry() const {return _entry;}

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

private:
	void updateBoundingRect();

	WayPoint _entry;
	QRectF _boundingRect;
};

#endif // POIITEM_H
