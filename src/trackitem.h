#ifndef TRACKITEM_H
#define TRACKITEM_H

#include <QDateTime>
#include <QPen>
#include "track.h"
#include "pathitem.h"
#include "units.h"

class Map;

class TrackItem : public PathItem
{
	Q_OBJECT

public:
	TrackItem(const Track &track, Map *map, QGraphicsItem *parent = 0);

	void setUnits(Units units);

private:
	QString toolTip(Units units) const;

	QString _name;
	QString _desc;
	QDateTime _date;
	qreal _time;
	qreal _movingTime;
};

#endif // TRACKITEM_H
