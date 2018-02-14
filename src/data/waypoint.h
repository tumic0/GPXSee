#ifndef WAYPOINT_H
#define WAYPOINT_H

#include <QString>
#include <QDateTime>
#include <QHash>
#include <QDebug>
#include <cmath>
#include "common/coordinates.h"

class Waypoint
{
public:
	Waypoint() {_elevation = NAN;}
	Waypoint(const Coordinates &coordinates) : _coordinates(coordinates)
	  {_elevation = NAN;}

	const Coordinates &coordinates() const {return _coordinates;}
	const QString &name() const {return _name;}
	const QString &description() const {return _description;}
	const QDateTime &timestamp() const {return _timestamp;}
	qreal elevation() const {return _elevation;}

	void setCoordinates(const Coordinates &coordinates)
	  {_coordinates = coordinates;}
	void setName(const QString &name) {_name = name;}
	void setDescription(const QString &description)
	  {_description = description;}
	void setTimestamp(const QDateTime &timestamp) {_timestamp = timestamp;}
	void setElevation(qreal elevation) {_elevation = elevation;}

	bool hasElevation() const {return !std::isnan(_elevation);}

	bool operator==(const Waypoint &other) const
	  {return this->_name == other._name
	  && this->_coordinates == other._coordinates;}

private:
	Coordinates _coordinates;
	QString _name;
	QString _description;
	QDateTime _timestamp;
	qreal _elevation;
};

inline uint qHash(const Waypoint &key)
{
	return ::qHash(key.name());
}

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const Waypoint &waypoint)
{
	dbg.nospace() << "Waypoint(" << waypoint.coordinates() << ", "
	  << waypoint.name() << ", " << waypoint.description() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

Q_DECLARE_TYPEINFO(Waypoint, Q_MOVABLE_TYPE);

#endif // WAYPOINT_H
