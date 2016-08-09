#ifndef GPX_H
#define GPX_H

#include <QVector>
#include <QList>
#include <QPointF>
#include <QString>
#include "waypoint.h"
#include "track.h"
#include "route.h"
#include "parser.h"

class GPX
{
public:
	GPX();
	~GPX();

	bool loadFile(const QString &fileName);
	const QString &errorString() const {return _error;}
	int errorLine() const {return _errorLine;}

	const QList<Track*> &tracks() const {return _tracks;}
	const QList<Route*> &routes() const {return _routes;}
	const QList<Waypoint> &waypoints() const {return _waypoint_data;}

private:
	Parser _parser;
	QString _error;
	int _errorLine;

	QList<Track*> _tracks;
	QList<Route*> _routes;

	QList<QVector<Trackpoint> > _track_data;
	QList<QVector<Waypoint> > _route_data;
	QList<Waypoint> _waypoint_data;
};

#endif // GPX_H
