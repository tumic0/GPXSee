#ifndef TRACKPOINT_H
#define TRACKPOINT_H

#include <QPointF>
#include <QDateTime>

struct TrackPoint
{
	QPointF coordinates;
	QDateTime timestamp;
	qreal elevation;
	qreal geoidheight;
	qreal speed;

	TrackPoint() {elevation = 0; geoidheight = 0; speed = -1;}
};

#endif // TRACKPOINT_H
