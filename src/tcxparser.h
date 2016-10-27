#ifndef TCXPARSER_H
#define TCXPARSER_H

#include <QXmlStreamReader>
#include "parser.h"


class TCXParser : public Parser
{
public:
	TCXParser(QList<QVector<Trackpoint> > &tracks,
	  QList<QVector<Waypoint> > &routes, QList<Waypoint> &waypoints)
	  : Parser(tracks, routes, waypoints) {}
	~TCXParser() {}

	bool loadFile(QIODevice *device);
	QString errorString() const {return _reader.errorString();}
	int errorLine() const {return _reader.lineNumber();}

private:
	bool parse();
	void tcx();
	void courses();
	void activities();
	void course();
	void activity();
	void lap();
	void trackpoints(QVector<Trackpoint> &track);
	void trackpointData(Trackpoint &trackpoint);
	void waypointData(Waypoint &waypoint);
	Coordinates position();
	qreal number();
	QDateTime time();

	QXmlStreamReader _reader;
};

#endif // TCXPARSER_H
