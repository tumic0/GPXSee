#ifndef PARSER_H
#define PARSER_H

#include <QString>
#include <QList>
#include <QFile>
#include "route.h"
#include "track.h"
#include "waypoint.h"


class Parser
{
public:
	virtual ~Parser() {}

	virtual bool parse(QFile *file, QList<TrackData> &tracks,
	  QList<RouteData> &routes, QList<Waypoint> &waypoints) = 0;
	virtual QString errorString() const = 0;
	virtual int errorLine() const = 0;
};

#endif // PARSER_H
