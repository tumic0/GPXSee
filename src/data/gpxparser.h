#ifndef GPXPARSER_H
#define GPXPARSER_H

#include <QXmlStreamReader>
#include "parser.h"


class GPXParser : public Parser
{
public:
	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &areas, QVector<Waypoint> &waypoints);
	QString errorString() const {return _reader.errorString();}
	int errorLine() const {return _reader.lineNumber();}

private:
	void gpx(QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &areas, QVector<Waypoint> &waypoints);
	void track(TrackData &track);
	void trackpoints(TrackData &track);
	void routepoints(RouteData &route, QList<TrackData> &tracks);
	void rpExtension(TrackData *autoRoute);
	void tpExtension(Trackpoint &trackpoint);
	void trkptExtensions(Trackpoint &trackpoint);
	void rteptExtensions(TrackData *autoRoute);
	void area(Area &area);
	void gpxExtensions(QList<Area> &areas);
	void trackpointData(Trackpoint &trackpoint);
	void waypointData(Waypoint &waypoint, TrackData *autoRoute = 0);
	qreal number();
	QDateTime time();
	Coordinates coordinates();

	QXmlStreamReader _reader;
};

#endif // GPXPARSER_H
