#ifndef PLANEITEM_H
#define PLANEITEM_H

#include "common/rectc.h"
#include "graphicsscene.h"

class Map;

class PlaneItem : public QObject, public GraphicsItem
{
	Q_OBJECT

public:
	PlaneItem(GraphicsItem *parent = 0) : GraphicsItem(parent) {}

	virtual RectC bounds() const = 0;
	virtual void setMap(Map *map) = 0;

	virtual void setColor(const QColor &color) = 0;
	virtual void setOpacity(qreal opacity) = 0;
	virtual void setWidth(qreal width) = 0;
	virtual void setPenStyle(Qt::PenStyle style) = 0;
	virtual void setDigitalZoom(int zoom) = 0;
	virtual void updateStyle() {}

	virtual const QColor color() const = 0;
	virtual const QString &name() const = 0;

public slots:
	virtual void hover(bool hvr) = 0;
};

#endif // PLANEITEM_H
