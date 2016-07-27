#ifndef TRACKITEM_H
#define TRACKITEM_H

#include <QGraphicsPathItem>
#include <QDateTime>
#include "units.h"
#include "track.h"

class TrackItem : public QGraphicsPathItem
{
public:
	TrackItem(const Track &track);

	QPainterPath shape() const {return _shape;}
	void setScale(qreal scale);

	void setUnits(enum Units units);
	void setColor(const QColor &color);

private:
	void updateShape();
	QString toolTip();

	QPainterPath _shape;

	Units _units;
	QDateTime _date;
	qreal _time;
	qreal _distance;
};

#endif // TRACKITEM_H
