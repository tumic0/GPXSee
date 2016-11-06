#ifndef GPXPARSER_H
#define GPXPARSER_H

#include <QXmlStreamReader>
#include "parser.h"


class GPXParser : public Parser
{
public:
	GPXParser(QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Waypoint> &waypoints) : Parser(tracks, routes, waypoints) {}
	~GPXParser() {}

	bool loadFile(QFile *file);
	QString errorString() const {return _reader.errorString();}
	int errorLine() const {return _reader.lineNumber();}

private:
	bool parse();
	void gpx();
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
