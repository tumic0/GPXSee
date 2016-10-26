#ifndef TRACKPOINT_H
#define TRACKPOINT_H

#include <QDateTime>
#include <QDebug>
#include <cmath>
#include "coordinates.h"

class Trackpoint
{
public:
	Trackpoint()
	  {_elevation = NAN; _speed = NAN; _heartRate = NAN; _temperature = NAN;}
	Trackpoint(const Coordinates &coordinates) : _coordinates(coordinates)
	  {_elevation = NAN; _speed = NAN; _heartRate = NAN; _temperature = NAN;}

	const Coordinates &coordinates() const {return _coordinates;}
	const QDateTime &timestamp() const {return _timestamp;}
	qreal elevation() const {return _elevation;}
	qreal speed() const {return _speed;}
	qreal heartRate() const {return _heartRate;}
	qreal temperature() const {return _temperature;}

	void setCoordinates(const Coordinates &coordinates)
	  {_coordinates = coordinates;}
	void setTimestamp(const QDateTime &timestamp) {_timestamp = timestamp;}
	void setElevation(qreal elevation) {_elevation = elevation;}
	void setSpeed(qreal speed) {_speed = speed;}
	void setHeartRate(qreal heartRate) {_heartRate = heartRate;}
	void setTemperature(qreal temperature) {_temperature = temperature;}

	bool hasTimestamp() const {return !_timestamp.isNull();}
	bool hasElevation() const {return !std::isnan(_elevation);}
	bool hasSpeed() const {return !std::isnan(_speed);}
	bool hasHeartRate() const {return !std::isnan(_heartRate);}
	bool hasTemperature() const {return !std::isnan(_temperature);}

private:
	Coordinates _coordinates;
	QDateTime _timestamp;
	qreal _elevation;
	qreal _speed;
	qreal _heartRate;
	qreal _temperature;
};

QDebug operator<<(QDebug dbg, const Trackpoint &trackpoint);

#endif // TRACKPOINT_H
