#ifndef DATA_H
#define DATA_H

#include <QList>
#include <QMultiMap>
#include <QString>
#include <QStringList>
#include "waypoint.h"
#include "track.h"
#include "route.h"
#include "parser.h"

class Data
{
public:
    Data();

    bool load(const QString &fileName, bool tryUnknown = true);
    bool load(const QUrl &url);
    bool save(const QString &fileName);

	bool isValid() const {return _valid;}
	const QString &errorString() const {return _errorString;}
	int errorLine() const {return _errorLine;}

    const QList<Track> &tracks() const {return _tracks;}
	const QList<Route> &routes() const {return _routes;}
	const QVector<Waypoint> &waypoints() const {return _waypoints;}
	const QList<Area> &areas() const {return _polygons;}

	static QString formats();
	static QStringList filter();

    bool addTrack(const Track& track = Track(TrackData()));
    bool addTrackPoint(const Trackpoint& point);

private:
    bool processData(QList<TrackData> &trackData, QList<RouteData> &routeData);
    bool GenerateData(QList<TrackData> &trackData, QList<RouteData> &routeData);

	bool _valid;
	QString _errorString;
	int _errorLine;

	QList<Track> _tracks;
	QList<Route> _routes;
	QList<Area> _polygons;
	QVector<Waypoint> _waypoints;

	static QMultiMap<QString, Parser*> _parsers;
#ifdef Q_OS_ANDROID
	static bool _permChecked;
#endif // Q_OS_ANDROID
};

#endif // DATA_H
