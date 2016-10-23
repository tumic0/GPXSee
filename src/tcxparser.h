#ifndef TCXPARSER_H
#define TCXPARSER_H

#include <QXmlStreamReader>
#include <QVector>
#include "parser.h"


class TCXParser : public Parser
{
public:
	TCXParser(QList<QVector<Trackpoint> > &tracks,
	  QList<QVector<Waypoint> > &routes, QList<Waypoint> &waypoints)
	  : Parser(tracks, routes, waypoints) {_track = 0; _route = 0;}
	~TCXParser() {}

	bool loadFile(QIODevice *device);
	QString errorString() const {return _reader.errorString();}
	int errorLine() const {return _reader.lineNumber();}
	const char *name() const {return "TCX";}

private:
	bool parse();
	void tcx();
	void courses();
	void activities();
	void course();
	void activity();
	void lap();
	void trackpoints();
	void routepoints();
	void trackpointData();
	void routepointData();
	void waypointData();
	QPointF position();

	QXmlStreamReader _reader;
	QVector<Trackpoint> *_track;
	QVector<Waypoint> *_route;
};

#endif // TCXPARSER_H
