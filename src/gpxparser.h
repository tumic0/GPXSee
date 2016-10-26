#ifndef GPXPARSER_H
#define GPXPARSER_H

#include <QXmlStreamReader>
#include <QVector>
#include "parser.h"


class GPXParser : public Parser
{
public:
	GPXParser(QList<QVector<Trackpoint> > &tracks,
	  QList<QVector<Waypoint> > &routes, QList<Waypoint> &waypoints)
	  : Parser(tracks, routes, waypoints) {_track = 0; _route = 0;}
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
	void track();
	void trackpoints();
	void routepoints();
	void tpExtension();
	void extensions();
	void trackpointData();
	void waypointData(Waypoint &waypoint);
	qreal number();
	QDateTime time();
	Coordinates coordinates();

	void handleWaypointData(DataType type, Waypoint &waypoint);
	void handleTrackpointData(DataType type);

	QXmlStreamReader _reader;
	QVector<Trackpoint> *_track;
	QVector<Waypoint> *_route;
};

#endif // GPXPARSER_H
