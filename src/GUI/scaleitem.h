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
	void setUnits(Units units);
	void setDigitalZoom(qreal zoom);

private:
	struct Tick {
		double value;
		QRect boundingBox;
	};

	void computeScale();
	void updateCache();

	qreal _res;
	qreal _width;
	qreal _length;
	Units _units;
	bool _scale;
	qreal _digitalZoom;
	QRectF _boundingRect;
	QFont _font;
	QVector<Tick> _ticks;
	QRect _unitsBB;
	QString _unitsStr;
};

#endif // SCALEITEM_H
