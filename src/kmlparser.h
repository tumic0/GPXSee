#ifndef KMLPARSER_H
#define KMLPARSER_H

#include <QXmlStreamReader>
#include <QDateTime>
#include "parser.h"

class KMLParser : public Parser
{
public:
	KMLParser(QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Waypoint> &waypoints) : Parser(tracks, routes, waypoints) {}
	~KMLParser() {}

	bool loadFile(QFile *file);
	QString errorString() const {return _reader.errorString();}
	int errorLine() const {return _reader.lineNumber();}

private:
	bool parse();
	void kml();
	void document();
	void folder();
	void placemark();
	void multiGeometry(const QString &name, const QString &desc,
	  const QDateTime timestamp);
	void track(TrackData &track);
	void lineString(TrackData &track);
	void point(Waypoint &waypoint);
	bool pointCoordinates(Waypoint &waypoint);
	bool lineCoordinates(TrackData &track);
	bool coord(Trackpoint &trackpoint);
	QDateTime timeStamp();
	qreal number();
	QDateTime time();

	QXmlStreamReader _reader;
};

#endif // KMLPARSER_H
