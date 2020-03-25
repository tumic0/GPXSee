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

	const RouteData &data() const {return _data;}
	Path path() const;
	GraphPair elevation() const;
	qreal distance() const;

	const QString &name() const {return _data.name();}
	const QString &description() const {return _data.description();}
	const QString &comment() const {return _data.comment();}
	const QVector<Link> &links() const {return _data.links();}

	bool isValid() const {return _data.size() >= 2;}

	static void useDEM(bool use) {_useDEM = use;}
	static void showSecondaryElevation(bool show)
	  {_show2ndElevation = show;}

private:
	Graph gpsElevation() const;
	Graph demElevation() const;

	RouteData _data;
	QVector<qreal> _distance;

	static bool _useDEM;
	static bool _show2ndElevation;
};

#endif // ROUTE_H
