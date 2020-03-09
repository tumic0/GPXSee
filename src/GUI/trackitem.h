#ifndef TRACKITEM_H
#define TRACKITEM_H

#include <QDateTime>
#include <QPen>
#include "data/track.h"
#include "pathitem.h"
#include "units.h"
#include "graphicsscene.h"

class Map;

class TrackItem : public PathItem
{
	Q_OBJECT

public:
	TrackItem(const Track &track, Map *map, QGraphicsItem *parent = 0);

	QString info() const;

private:
	QString _name;
	QString _desc;
	QString _comment;
	QVector<Link> _links;
	QDateTime _date;
	qreal _time;
	qreal _movingTime;
};

#endif // TRACKITEM_H
