#ifndef DATA_H
#define DATA_H

#include <QVector>
#include <QList>
#include <QHash>
#include <QPointF>
#include <QString>
#include "waypoint.h"
#include "track.h"
#include "route.h"
#include "parser.h"


class Data
{
public:
	Data();
	~Data();

	bool loadFile(const QString &fileName);
	const QString &errorString() const {return _errorString;}
	int errorLine() const {return _errorLine;}

	const QList<Track*> &tracks() const {return _tracks;}
	const QList<Route*> &routes() const {return _routes;}
	const QList<Waypoint> &waypoints() const {return _waypointData;}

private:
	void createData();

	QString _errorString;
	int _errorLine;
	QHash<QString, Parser*> _parsers;

	QList<Track*> _tracks;
	QList<Route*> _routes;

	QList<TrackData> _trackData;
	QList<RouteData> _routeData;
	QList<Waypoint> _waypointData;
};

#endif // DATA_H
