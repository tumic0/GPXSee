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
		_elevation = NAN; _geoidHeight = 0; _speed = NAN; _heartRate = NAN;
		_temperature = NAN;
	}
	Trackpoint(const QPointF &coordinates) {_coordinates = coordinates;}

	const QPointF &coordinates() const {return _coordinates;}
	const QDateTime &timestamp() const {return _timestamp;}
	qreal elevation() const {return _elevation;}
	qreal geoidHeight() const {return _geoidHeight;}
	qreal speed() const {return _speed;}
	qreal heartRate() const {return _heartRate;}
	qreal temperature() const {return _temperature;}

	void setCoordinates(const QPointF &coordinates)
	  {_coordinates = coordinates;}
	void setTimestamp(const QDateTime &timestamp) {_timestamp = timestamp;}
	void setElevation(qreal elevation) {_elevation = elevation;}
	void setGeoidHeight(qreal geoidHeight) {_geoidHeight = geoidHeight;}
	void setSpeed(qreal speed) {_speed = speed;}
	void setHeartRate(qreal heartRate) {_heartRate = heartRate;}
	void setTemperature(qreal temperature) {_temperature = temperature;}

	bool hasTimestamp() const {return !_timestamp.isNull();}
	bool hasElevation() const {return !std::isnan(_elevation);}
	bool hasSpeed() const {return !std::isnan(_speed);}
	bool hasHeartRate() const {return !std::isnan(_heartRate);}
	bool hasTemperature() const {return !std::isnan(_temperature);}

private:
	QPointF _coordinates;
	QDateTime _timestamp;
	qreal _elevation;
	qreal _geoidHeight;
	qreal _speed;
	qreal _heartRate;
	qreal _temperature;
};

QDebug operator<<(QDebug dbg, const Trackpoint &trackpoint);

#endif // TRACKPOINT_H
