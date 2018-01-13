#ifndef GPXPARSER_H
#define GPXPARSER_H

#include <QXmlStreamReader>
#include "parser.h"


class GPXParser : public Parser
{
public:
	bool parse(QFile *file, QList<TrackData> &tracks,
	  QList<RouteData> &routes, QList<Waypoint> &waypoints);
	QString errorString() const {return _reader.errorString();}
	int errorLine() const {return _reader.lineNumber();}

private:
	void gpx(QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Waypoint> &waypoints);
	void track(TrackData &track);
	void trackpoints(TrackData &track);
	void routepoints(RouteData &route);
	void tpExtension(Trackpoint &trackpoint);
	void extensions(Trackpoint &trackpoint);
	void trackpointData(Trackpoint &trackpoint);
	void waypointData(Waypoint &waypoint);
	qreal number();
	QDateTime time();
	Coordinates coordinates();

	QXmlStreamReader _reader;
};

#endif // GPXPARSER_H
