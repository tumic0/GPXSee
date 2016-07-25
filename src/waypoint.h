#ifndef WAYPOINT_H
#define WAYPOINT_H

#include <QPointF>
#include <QString>
#include <QHash>

class Waypoint
{
public:
	Waypoint() {}
	Waypoint(const QPointF &coordinates, const QString &name,
	  const QString &description)
	  : _coordinates(coordinates), _name(name), _description(description) {}

	const QPointF &coordinates() const {return _coordinates;}
	const QString &name() const {return _name;}
	const QString &description() const {return _description;}
	void setCoordinates(const QPointF &coordinates)
	  {_coordinates = coordinates;}
	void setName(const QString &name)
	  {_name = name;}
	void setDescription(const QString &description)
	  {_description = description;}

	bool operator==(const Waypoint &other) const
	  {return this->_name == other._name
	  && this->_coordinates == other._coordinates;}

private:
	QPointF _coordinates;
	QString _name;
	QString _description;
};

inline uint qHash(const Waypoint &key)
{
	return ::qHash(key.name());
}

#endif // WAYPOINT_H
