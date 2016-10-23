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
	const char *name() const {return "GPX";}

private:
	enum DataType {
		Name, Description, Elevation, Time, Geoidheight, Speed, HeartRate,
		Temperature
	};

	bool parse();
	void gpx();
	void track();
	void trackpoints();
	void routepoints();
	void tpExtension();
	void extensions();
	void trackpointData();
	void routepointData();
	void waypointData();

	void handleWaypointAttributes(const QXmlStreamAttributes &attr);
	void handleWaypointData(DataType type, const QString &value);
	void handleTrackpointAttributes(const QXmlStreamAttributes &attr);
	void handleTrackpointData(DataType type, const QString &value);
	void handleRoutepointAttributes(const QXmlStreamAttributes &attr);
	void handleRoutepointData(DataType type, const QString &value);

	QXmlStreamReader _reader;
	QVector<Trackpoint> *_track;
	QVector<Waypoint> *_route;
};

#endif // GPXPARSER_H
