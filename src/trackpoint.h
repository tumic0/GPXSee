#ifndef TRACKPOINT_H
#define TRACKPOINT_H

#include <QPointF>
#include <QDateTime>
#include <cmath>

struct Trackpoint
{
	QPointF coordinates;
	QDateTime timestamp;
	qreal elevation;
	qreal geoidheight;
	qreal speed;
	qreal heartRate;

	Trackpoint() {
		elevation = NAN;
		geoidheight = 0;
		speed = NAN;
		heartRate = NAN;
	}
};

#endif // TRACKPOINT_H
