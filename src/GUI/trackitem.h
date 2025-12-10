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

	ToolTip info(bool extended) const;

private:
	qreal _time;
	qreal _movingTime;
	bool _video;
};

#endif // TRACKITEM_H
