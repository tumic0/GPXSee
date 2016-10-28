#ifndef TRACKITEM_H
#define TRACKITEM_H

#include <QDateTime>
#include <QPen>
#include "track.h"
#include "pathitem.h"


class TrackItem : public PathItem
{
	Q_OBJECT

public:
	TrackItem(const Track &track, QGraphicsItem *parent = 0);

	void setUnits(enum Units units);

private:
	QString toolTip();

	QString _name;
	QString _desc;
	QDateTime _date;
	qreal _time;
};

#endif // TRACKITEM_H
