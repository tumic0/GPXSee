#ifndef KMLPARSER_H
#define KMLPARSER_H

#include <QXmlStreamReader>
#include <QDateTime>
#include "parser.h"

class KMLParser : public Parser
{
public:
	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &areas, QVector<Waypoint> &waypoints);
	QString errorString() const {return _reader.errorString();}
	int errorLine() const {return _reader.lineNumber();}

private:
	void kml(QList<TrackData> &tracks, QList<Area> &areas,
	  QVector<Waypoint> &waypoints);
	void document(QList<TrackData> &tracks, QList<Area> &areas,
	  QVector<Waypoint> &waypoints);
	void folder(QList<TrackData> &tracks, QList<Area> &areas,
	  QVector<Waypoint> &waypoints);
	void placemark(QList<TrackData> &tracks, QList<Area> &areas,
	  QVector<Waypoint> &waypoints);
	void multiGeometry(QList<TrackData> &tracks, QList<Area> &areas,
	  QVector<Waypoint> &waypoints, const QString &name, const QString &desc,
	  const QDateTime &timestamp);
	void track(SegmentData &segment);
	void multiTrack(TrackData &t);
	void lineString(SegmentData &segment);
	void linearRing(QVector<Coordinates> &coordinates);
	void boundary(QVector<Coordinates> &coordinates);
	void polygon(Area &area);
	void point(Waypoint &waypoint);
	bool pointCoordinates(Waypoint &waypoint);
	bool lineCoordinates(SegmentData &segment);
	bool polygonCoordinates(QVector<Coordinates> &points);
	bool coord(Trackpoint &trackpoint);
	void extendedData(SegmentData &segment, int start);
	void schemaData(SegmentData &segment, int start);
	void heartRate(SegmentData &segment, int start);
	void cadence(SegmentData &segment, int start);
	void speed(SegmentData &segment, int start);
	void temperature(SegmentData &segment, int start);
	QDateTime timeStamp();
	qreal number();
	QDateTime time();

	QXmlStreamReader _reader;
};

#endif // KMLPARSER_H
