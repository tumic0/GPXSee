#ifndef TRACKITEM_H
#define TRACKITEM_H

#include <QDateTime>
#include "pathitem.h"

class Map;
class Track;

class TrackItem : public PathItem
{
	Q_OBJECT

public:
	TrackItem(const Track &track, Map *map, QGraphicsItem *parent = 0);

	ToolTip info() const;
	QDateTime date() const {return _date;}

private:
	QDateTime _date;
	qreal _time;
	qreal _movingTime;
};

#endif // TRACKITEM_H
