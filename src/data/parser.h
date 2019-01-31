#ifndef PARSER_H
#define PARSER_H

#include <QString>
#include <QList>
#include <QVector>
#include <QFile>
#include "trackdata.h"
#include "routedata.h"
#include "waypoint.h"
#include "area.h"


class Parser
{
public:
	virtual ~Parser() {}

	virtual bool parse(QFile *file, QList<TrackData> &tracks,
	  QList<RouteData> &routes, QList<Area> &polygons,
	  QVector<Waypoint> &waypoints) = 0;
	virtual QString errorString() const = 0;
	virtual int errorLine() const = 0;
};

#endif // PARSER_H
