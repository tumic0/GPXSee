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

	const RouteData &routeData() const {return _data;}
	const QVector<qreal> &distanceData() const {return _distance;}

	Graph elevation() const;

	qreal distance() const;

	bool isNull() const {return (_data.count() < 2);}

private:
	const RouteData &_data;
	QVector<qreal> _distance;
};

#endif // ROUTE_H
