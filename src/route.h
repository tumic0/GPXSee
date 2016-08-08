#ifndef ROUTE_H
#define ROUTE_H

#include <QVector>
#include "waypoint.h"

class Route
{
public:
	Route(const QVector<Waypoint> &data);

	const QVector<Waypoint> &route() const {return _data;}
	void elevationGraph(QVector<QPointF> &graph) const;
	qreal distance() const {return _dd.last();}

	bool isNull() const {return (_data.count() < 2) ? true : false;}

private:
	const QVector<Waypoint> &_data;
	QVector<qreal> _dd;
};

#endif // ROUTE_H
