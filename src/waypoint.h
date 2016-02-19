#ifndef WAYPOINT_H
#define WAYPOINT_H

#include <QPointF>
#include <QString>
#include <QHash>

class Waypoint
{
public:
	Waypoint() {}
	Waypoint(const QPointF &coordinates, const QString &description)
	  : _coordinates(coordinates), _description(description) {}

	const QPointF &coordinates() const {return _coordinates;}
	const QString &description() const {return _description;}
	void setCoordinates(const QPointF &coordinates)
	  {_coordinates = coordinates;}
	void setDescription(const QString &description)
	  {_description = description;}

	bool operator==(const Waypoint &other) const
	  {return this->_description == other._description
	  && this->_coordinates == other._coordinates;}

private:
	QPointF _coordinates;
	QString _description;
};

inline uint qHash(const Waypoint &key)
{
	return ::qHash(key.description());
}

#endif // WAYPOINT_H
