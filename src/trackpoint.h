#ifndef TRACKPOINT_H
#define TRACKPOINT_H

#include <QPointF>
#include <QDateTime>
#include <cmath>

class Trackpoint
{
public:
	Trackpoint() {
		elevation = NAN;
		geoidheight = 0;
		speed = NAN;
		heartRate = NAN;
	}

	bool hasElevation() const {return !std::isnan(elevation);}
	bool hasSpeed() const {return !std::isnan(speed);}
	bool hasHeartRate() const {return !std::isnan(heartRate);}

	QPointF coordinates;
	QDateTime timestamp;
	qreal elevation;
	qreal geoidheight;
	qreal speed;
	qreal heartRate;
};

#endif // TRACKPOINT_H
