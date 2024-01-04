#ifndef MOTIONINFOITEM_H
#define MOTIONINFOITEM_H

#include <QGraphicsItem>
#include <QFont>
#include "units.h"

class MotionInfoItem : public QGraphicsItem
{
public:
	MotionInfoItem(QGraphicsItem *parent = 0);

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void setInfo(qreal bearing, qreal speed, qreal verticalSpeed);
	void setUnits(Units units);
	void setDigitalZoom(qreal zoom);
	void setColor(const QColor &color);
	void setBackgroundColor(const QColor &color);
	void drawBackground(bool draw);

private:
	void updateBoundingRect();
	QString speed(const QLocale &l) const;
	QString verticalSpeed(const QLocale &l) const;
	QString text() const;

	qreal _bearing, _speed, _verticalSpeed;
	Units _units;
	QRectF _boundingRect;
	QFont _font;
	qreal _digitalZoom;
	QColor _color, _bgColor;
	bool _drawBackground;
};

#endif // MOTIONINFOITEM_H
