#ifndef TRACKPOINT_H
#define TRACKPOINT_H

#include <QDateTime>
#include <QDebug>
#include <cmath>
#include "common/coordinates.h"

class Trackpoint
{
public:
	Trackpoint()
	  {_elevation = NAN; _speed = NAN; _heartRate = NAN; _temperature = NAN;
	  _cadence = NAN; _power = NAN; _ratio = NAN;}
	Trackpoint(const Coordinates &coordinates) : _coordinates(coordinates)
	  {_elevation = NAN; _speed = NAN; _heartRate = NAN; _temperature = NAN;
	  _cadence = NAN; _power = NAN; _ratio = NAN;}

	const Coordinates &coordinates() const {return _coordinates;}
	Coordinates &rcoordinates() {return _coordinates;}
	const QDateTime &timestamp() const {return _timestamp;}
	qreal elevation() const {return _elevation;}
	qreal speed() const {return _speed;}
	qreal heartRate() const {return _heartRate;}
	qreal temperature() const {return _temperature;}
	qreal cadence() const {return _cadence;}
	qreal power() const {return _power;}
	qreal ratio() const {return _ratio;}

	void setCoordinates(const Coordinates &coordinates)
	  {_coordinates = coordinates;}
	void setTimestamp(const QDateTime &timestamp) {_timestamp = timestamp;}
	void setElevation(qreal elevation) {_elevation = elevation;}
	void setSpeed(qreal speed) {_speed = speed;}
	void setHeartRate(qreal heartRate) {_heartRate = heartRate;}
	void setTemperature(qreal temperature) {_temperature = temperature;}
	void setCadence(qreal cadence) {_cadence = cadence;}
	void setPower(qreal power) {_power = power;}
	void setRatio(qreal ratio) {_ratio = ratio;}

	bool hasTimestamp() const {return !_timestamp.isNull();}
	bool hasElevation() const {return !std::isnan(_elevation);}
	bool hasSpeed() const {return !std::isnan(_speed);}
	bool hasHeartRate() const {return !std::isnan(_heartRate);}
	bool hasTemperature() const {return !std::isnan(_temperature);}
	bool hasCadence() const {return !std::isnan(_cadence);}
	bool hasPower() const {return !std::isnan(_power);}
	bool hasRatio() const {return !std::isnan(_ratio);}

private:
	Coordinates _coordinates;
	QDateTime _timestamp;
	qreal _elevation;
	qreal _speed;
	qreal _heartRate;
	qreal _temperature;
	qreal _cadence;
	qreal _power;
	qreal _ratio;
};

Q_DECLARE_TYPEINFO(Trackpoint, Q_MOVABLE_TYPE);

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const Trackpoint &trackpoint)
{
	dbg.nospace() << "Trackpoint(" << trackpoint.coordinates() << ", "
	  << trackpoint.timestamp() << ", " << trackpoint.elevation() << ", "
	  << trackpoint.speed() << ", " << trackpoint.heartRate() << ", "
	  << trackpoint.temperature() << ", " << trackpoint.cadence() << ", "
	  << trackpoint.power() << ", " << trackpoint.ratio() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // TRACKPOINT_H
