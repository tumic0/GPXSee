#ifndef PARSER_H
#define PARSER_H

#include <QXmlStreamReader>
#include <QVector>
#include <QList>
#include "trackpoint.h"
#include "waypoint.h"


class Parser
{
public:
	Parser(QList<QVector<Trackpoint> > &tracks, QList<Waypoint> &waypoints)
	  : _tracks(tracks), _waypoints(waypoints) {_track = 0;}
	bool loadFile(QIODevice *device);
	QString errorString() const {return _reader.errorString();}
	int errorLine() const {return _reader.lineNumber();}

private:
	bool parse();
	void gpx();
	void track();
	void trackPoints();
	void extensions();
	void trackPointData();
	void wayPointData();

	void handleWayPointAttributes(const QXmlStreamAttributes &attr);
	void handleWayPointData(QStringRef element, const QString &value);
	void handleTrekPointAttributes(const QXmlStreamAttributes &attr);
	void handleTrekPointData(QStringRef element, const QString &value);
	void handleExtensionData(QStringRef element, const QString &value);

	QXmlStreamReader _reader;
	QList<QVector<Trackpoint> > &_tracks;
	QList<Waypoint> &_waypoints;
	QVector<Trackpoint> *_track;
};

#endif // PARSER_H
