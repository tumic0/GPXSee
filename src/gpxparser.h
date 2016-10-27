#ifndef GPXPARSER_H
#define GPXPARSER_H

#include <QXmlStreamReader>
#include "parser.h"


class GPXParser : public Parser
{
public:
	GPXParser(QList<QVector<Trackpoint> > &tracks,
	  QList<QVector<Waypoint> > &routes, QList<Waypoint> &waypoints)
	  : Parser(tracks, routes, waypoints) {}
	~GPXParser() {}

	bool loadFile(QIODevice *device);
	QString errorString() const {return _reader.errorString();}
	int errorLine() const {return _reader.lineNumber();}

private:
	enum DataType {
		Name, Description, Elevation, Time, Speed, HeartRate, Temperature
	};

	bool parse();
	void gpx();
	void track(QVector<Trackpoint> &track);
	void trackpoints(QVector<Trackpoint> &track);
	void routepoints(QVector<Waypoint> &route);
	void tpExtension(Trackpoint &trackpoint);
	void extensions(Trackpoint &trackpoint);
	void trackpointData(Trackpoint &trackpoint);
	void waypointData(Waypoint &waypoint);
	qreal number();
	QDateTime time();
	Coordinates coordinates();

	void handleWaypointData(DataType type, Waypoint &waypoint);
	void handleTrackpointData(DataType type, Trackpoint &trackpoint);

	QXmlStreamReader _reader;
};

#endif // GPXPARSER_H
