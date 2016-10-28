#ifndef ROUTE_H
#define ROUTE_H

#include <QVector>
#include "routedata.h"
#include "graph.h"

class Route
{
public:
	Route(const RouteData &data);

	const RouteData &route() const {return _data;}
	Graph elevation() const;

	qreal distance() const;

	bool isNull() const {return (_data.count() < 2);}

private:
	const RouteData &_data;
	QVector<qreal> _distance;
};

#endif // ROUTE_H
