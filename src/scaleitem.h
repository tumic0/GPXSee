#ifndef SCALEITEM_H
#define SCALEITEM_H

#include <QGraphicsItem>
#include "units.h"

class ScaleItem : public QGraphicsItem
{
public:
	ScaleItem(QGraphicsItem *parent = 0);

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void setResolution(qreal res);
	void setUnits(enum Units units);
	void setDigitalZoom(qreal zoom);

private:
	void updateBoundingRect();
	void computeScale();
	QString units() const;

	qreal _res;
	qreal _width;
	qreal _length;
	Units _units;
	bool _scale;

	qreal _digitalZoom;

	QRectF _boundingRect;
};

#endif // SCALEITEM_H
