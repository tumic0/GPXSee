#ifndef ROUTE_H
#define ROUTE_H

#include <QVector>
#include "waypoint.h"
#include "graph.h"

class Route
{
public:
	Route(const QVector<Waypoint> &data);

	const QVector<Waypoint> &route() const {return _data;}
	Graph elevation() const;

	qreal distance() const;

	bool isNull() const {return (_data.count() < 2);}

private:
	const QVector<Waypoint> &_data;
	QVector<qreal> _distance;
};

#endif // ROUTE_H
