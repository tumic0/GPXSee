#ifndef WAYPOINT_H
#define WAYPOINT_H

#include <QPointF>
#include <QString>
#include <QDateTime>
#include <QHash>
#include <QDebug>
#include <cmath>

class Waypoint
{
public:
	Waypoint() {_elevation = NAN; _geoidHeight = 0;}
	Waypoint(const QPointF &coordinates)
	  : _coordinates(coordinates) {_elevation = NAN; _geoidHeight = 0;}

	const QPointF &coordinates() const {return _coordinates;}
	const QString &name() const {return _name;}
	const QString &description() const {return _description;}
	const QDateTime &timestamp() const {return _timestamp;}
	qreal elevation() const {return _elevation;}
	qreal geoidHeight() const {return _geoidHeight;}

	void setCoordinates(const QPointF &coordinates)
	  {_coordinates = coordinates;}
	void setName(const QString &name) {_name = name;}
	void setDescription(const QString &description)
	  {_description = description;}
	void setTimestamp(const QDateTime &timestamp) {_timestamp = timestamp;}
	void setElevation(qreal elevation) {_elevation = elevation;}
	void setGeoidHeight(qreal geoidHeight) {_geoidHeight = geoidHeight;}

	bool hasElevation() const {return !std::isnan(_elevation);}

	bool operator==(const Waypoint &other) const
	  {return this->_name == other._name
	  && this->_coordinates == other._coordinates;}

private:
	QPointF _coordinates;
	QString _name;
	QString _description;
	QDateTime _timestamp;
	qreal _elevation;
	qreal _geoidHeight;
};

inline uint qHash(const Waypoint &key)
{
	return ::qHash(key.name());
}

QDebug operator<<(QDebug dbg, const Waypoint &Waypoint);

#endif // WAYPOINT_H
