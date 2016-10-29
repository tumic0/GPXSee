#ifndef PARSER_H
#define PARSER_H

#include <QString>
#include <QList>
#include <QFile>
#include "trackdata.h"
#include "routedata.h"
#include "waypoint.h"


class Parser
{
public:
	Parser(QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Waypoint> &waypoints) : _tracks(tracks), _routes(routes),
	  _waypoints(waypoints) {}
	virtual ~Parser() {}

	virtual bool loadFile(QFile *file) = 0;
	virtual QString errorString() const = 0;
	virtual int errorLine() const = 0;

protected:
	QList<TrackData> &_tracks;
	QList<RouteData> &_routes;
	QList<Waypoint> &_waypoints;
};

#endif // PARSER_H
