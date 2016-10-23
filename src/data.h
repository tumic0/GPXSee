#ifndef DATA_H
#define DATA_H

#include <QVector>
#include <QList>
#include <QPointF>
#include <QString>
#include "waypoint.h"
#include "track.h"
#include "route.h"

class Parser;

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
	const QList<Waypoint> &waypoints() const {return _waypoint_data;}

private:
	QString _errorString;
	int _errorLine;
	QList<Parser*> _parsers;

	QList<Track*> _tracks;
	QList<Route*> _routes;

	QList<QVector<Trackpoint> > _track_data;
	QList<QVector<Waypoint> > _route_data;
	QList<Waypoint> _waypoint_data;
};

#endif // DATA_H
