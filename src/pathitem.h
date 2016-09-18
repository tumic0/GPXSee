#ifndef PATHITEM_H
#define PATHITEM_H

#include <QGraphicsObject>
#include "units.h"

class PathItem : public QGraphicsObject
{
	Q_OBJECT

public:
	PathItem(QGraphicsItem *parent = 0) : QGraphicsObject(parent) {}
	virtual void showMarker(bool show) = 0;

public slots:
	virtual void moveMarker(qreal distance) = 0;
};

#endif // PATHITEM_H
