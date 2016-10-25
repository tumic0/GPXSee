#ifndef KMLPARSER_H
#define KMLPARSER_H

#include <QXmlStreamReader>
#include <QVector>
#include <QDateTime>
#include "parser.h"

class KMLParser : public Parser
{
public:
	KMLParser(QList<QVector<Trackpoint> > &tracks,
	  QList<QVector<Waypoint> > &routes, QList<Waypoint> &waypoints)
	  : Parser(tracks, routes, waypoints) {_track = 0; _route = 0;}
	~KMLParser() {}

	bool loadFile(QIODevice *device);
	QString errorString() const {return _reader.errorString();}
	int errorLine() const {return _reader.lineNumber();}
	const char *name() const {return "KML";}

private:
	bool parse();
	void kml();
	void document();
	void folder();
	void placemark();
	void multiGeometry(const QString &name, const QString &desc,
	  const QDateTime timestamp);
	void lineString();
	void point(const QString &name, const QString &desc,
	  const QDateTime timestamp);
	bool pointCoordinates();
	bool lineCoordinates();
	QDateTime timeStamp();

	QXmlStreamReader _reader;
	QVector<Trackpoint> *_track;
	QVector<Waypoint> *_route;
};

#endif // KMLPARSER_H
