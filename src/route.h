#ifndef ROUTE_H
#define ROUTE_H

#include <QVector>
#include "routedata.h"
#include "graph.h"
#include "path.h"

class Route
{
public:
	Route(const RouteData &data);

	Path path() const;

	const QVector<Waypoint> &waypoints() const {return _data;}

	Graph elevation() const;

	qreal distance() const;

	const QString &name() const {return _data.name();}
	const QString &description() const {return _data.description();}

	bool isNull() const {return (_data.count() < 2);}

private:
	const RouteData &_data;
	QVector<qreal> _distance;
};

#endif // ROUTE_H
