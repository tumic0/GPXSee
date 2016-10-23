#ifndef PARSER_H
#define PARSER_H

#include <QString>
#include <QList>
#include "trackpoint.h"
#include "waypoint.h"

class QIODevice;

class Parser
{
public:
	Parser(QList<QVector<Trackpoint> > &tracks,
	  QList<QVector<Waypoint> > &routes, QList<Waypoint> &waypoints)
	  : _tracks(tracks), _routes(routes), _waypoints(waypoints) {}
	virtual ~Parser() {}

	virtual bool loadFile(QIODevice *device) = 0;
	virtual QString errorString() const = 0;
	virtual int errorLine() const = 0;
	virtual const char *name() const = 0;

protected:
	QList<QVector<Trackpoint> > &_tracks;
	QList<QVector<Waypoint> > &_routes;
	QList<Waypoint> &_waypoints;
};

#endif // PARSER_H
