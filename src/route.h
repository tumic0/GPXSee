#ifndef ROUTE_H
#define ROUTE_H

#include <QVector>
#include "waypoint.h"

class Route
{
public:
	Route(const QVector<Waypoint> &data);

	const QVector<Waypoint> &route() const {return _data;}
	QVector<QPointF> elevation() const;

	qreal distance() const;

	bool isNull() const {return _dd.isEmpty();}

private:
	const QVector<Waypoint> &_data;
	QVector<qreal> _dd;
};

#endif // ROUTE_H
