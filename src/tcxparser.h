#ifndef TCXPARSER_H
#define TCXPARSER_H

#include <QXmlStreamReader>
#include "parser.h"


class TCXParser : public Parser
{
public:
	TCXParser(QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Waypoint> &waypoints) : Parser(tracks, routes, waypoints) {}
	~TCXParser() {}

	bool loadFile(QFile *file);
	QString errorString() const {return _reader.errorString();}
	int errorLine() const {return _reader.lineNumber();}

private:
	bool parse();
	void tcx();
	void courses();
	void activities();
	void course(TrackData &track);
	void activity(TrackData &track);
	void lap(TrackData &track);
	void trackpoints(TrackData &track);
	void trackpointData(Trackpoint &trackpoint);
	void waypointData(Waypoint &waypoint);
	void extensions(Trackpoint &trackpoint);
	Coordinates position();
	qreal number();
	QDateTime time();

	void warning(const char *text) const;

	QXmlStreamReader _reader;
};

#endif // TCXPARSER_H
