#ifndef TRACKPOINT_H
#define TRACKPOINT_H

#include <QPointF>
#include <QDateTime>
#include <QDebug>
#include <cmath>

class Trackpoint
{
public:
	Trackpoint() {
		elevation = NAN;
		geoidheight = 0;
		speed = NAN;
		heartRate = NAN;
		temperature = NAN;
	}

	bool hasTimestamp() const {return !timestamp.isNull();}
	bool hasElevation() const {return !std::isnan(elevation);}
	bool hasSpeed() const {return !std::isnan(speed);}
	bool hasHeartRate() const {return !std::isnan(heartRate);}
	bool hasTemperature() const {return !std::isnan(temperature);}

	QPointF coordinates;
	QDateTime timestamp;
	qreal elevation;
	qreal geoidheight;
	qreal speed;
	qreal heartRate;
	qreal temperature;
};

QDebug operator<<(QDebug dbg, const Trackpoint &trackpoint);

#endif // TRACKPOINT_H
