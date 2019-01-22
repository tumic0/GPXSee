#ifndef DATA_H
#define DATA_H

#include <QList>
#include <QHash>
#include <QString>
#include <QStringList>
#include "waypoint.h"
#include "track.h"
#include "route.h"
#include "parser.h"


class Data
{
public:
	Data(const QString &fileName, bool poi = false);
	~Data();

	bool isValid() const {return _valid;}
	const QString &errorString() const {return _errorString;}
	int errorLine() const {return _errorLine;}

	const QList<Track*> &tracks() const {return _tracks;}
	const QList<Route*> &routes() const {return _routes;}
	const QVector<Waypoint> &waypoints() const {return _waypoints;}

	static QString formats();
	static QStringList filter();

	static void useDEM(bool use);

private:
	void processData();

	bool _valid;
	QString _errorString;
	int _errorLine;

	QList<Track*> _tracks;
	QList<Route*> _routes;
	QVector<Waypoint> _waypoints;

	QList<TrackData> _trackData;
	QList<RouteData> _routeData;

	static QHash<QString, Parser*> _parsers;
	static bool _useDEM;
};

#endif // DATA_H
