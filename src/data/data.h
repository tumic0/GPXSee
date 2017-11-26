#ifndef DATA_H
#define DATA_H

#include <QVector>
#include <QList>
#include <QHash>
#include <QPointF>
#include <QString>
#include <QStringList>
#include "waypoint.h"
#include "track.h"
#include "route.h"
#include "parser.h"


class Data : public QObject
{
	Q_OBJECT

public:
	Data(QObject *parent = 0) : QObject(parent), _errorLine(0) {}
	~Data();

	bool loadFile(const QString &fileName);
	const QString &errorString() const {return _errorString;}
	int errorLine() const {return _errorLine;}

	const QList<Track*> &tracks() const {return _tracks;}
	const QList<Route*> &routes() const {return _routes;}
	const QList<Waypoint> &waypoints() const {return _waypoints;}

	static QString formats();
	static QStringList filter();

private:
	void processData();

	QString _errorString;
	int _errorLine;

	QList<Track*> _tracks;
	QList<Route*> _routes;
	QList<Waypoint> _waypoints;

	QList<TrackData> _trackData;
	QList<RouteData> _routeData;

	static QHash<QString, Parser*> _parsers;
};

#endif // DATA_H
