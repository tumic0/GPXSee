#ifndef DATA_H
#define DATA_H

#include <QVector>
#include <QList>
#include <QHash>
#include <QPointF>
#include <QString>
#include <QStringList>
#include <QPair>
#include "waypoint.h"
#include "track.h"
#include "route.h"
#include "parser.h"

class Data : public QObject
{
	Q_OBJECT

public:
	typedef QPair<QDate, QDate> DateRange;

	Data(QObject *parent = 0);
	~Data();

	bool loadFile(const QString &fileName);
	const QString &errorString() const {return _errorString;}
	int errorLine() const {return _errorLine;}
	void clear();

	const QList<Track*> &tracks() const {return _tracks;}
	const QList<Route*> &routes() const {return _routes;}
	const QList<Waypoint> &waypoints() const {return _waypoints;}

	qreal getTracksTotalDistrance() const {return _trackDistance;}
	qreal getTracksTotalTime() const {return _trackTime;}
	qreal getTracksTotalMovingTime() const {return _trackMovingTime;}
	DateRange getTracksDateRange() const {return _trackDateRange;}

	qreal getRoutesTotalDistrance() const {return _routeDistance;}

	static QString formats();
	static QStringList filter();

private:

	void processData(QList<TrackData> &trackData, QList<RouteData> &routeData, QList<Waypoint> &waypoints);

	QString _errorString;
	int _errorLine;

	QList<Track*> _tracks;
	QList<Route*> _routes;
	QList<Waypoint> _waypoints;

	QList<TrackData> _trackData;
	QList<RouteData> _routeData;

	// TODO: Should be moved to TrackData
	qreal _trackDistance;
	qreal _trackTime;
	qreal _trackMovingTime;
	DateRange _trackDateRange;

	// TODO: Should be moved to RouteData
	qreal _routeDistance;

	static QHash<QString, Parser*> _parsers;
};

#endif // DATA_H
