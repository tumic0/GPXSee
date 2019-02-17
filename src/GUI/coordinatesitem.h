#ifndef COORDINATESITEM_H
#define COORDINATESITEM_H

#include <QGraphicsItem>
#include <QFont>
#include "common/coordinates.h"
#include "format.h"

class CoordinatesItem : public QGraphicsItem
{
public:
	CoordinatesItem(QGraphicsItem *parent = 0);

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void setCoordinates(const Coordinates &c);
	void setFormat(const CoordinatesFormat &format);

private:
	void updateBoundingRect();

	Coordinates _c;
	CoordinatesFormat _format;
	QRectF _boundingRect;
	QFont _font;
};

#endif // COORDINATESITEM_H
