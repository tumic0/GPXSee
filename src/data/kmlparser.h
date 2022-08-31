#ifndef KMLPARSER_H
#define KMLPARSER_H

#include <QXmlStreamReader>
#include <QDateTime>
#include "parser.h"

class QDir;

class KMLParser : public Parser
{
public:
	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &areas, QVector<Waypoint> &waypoints);
	QString errorString() const {return _reader.errorString();}
	int errorLine() const {return _reader.lineNumber();}

private:
	void kml(const QDir &dir, QList<TrackData> &tracks, QList<Area> &areas,
	  QVector<Waypoint> &waypoints);
	void document(const QDir &dir, QList<TrackData> &tracks, QList<Area> &areas,
	  QVector<Waypoint> &waypoints, QMap<QString, QPixmap> &icons);
	void folder(const QDir &dir, QList<TrackData> &tracks, QList<Area> &areas,
	  QVector<Waypoint> &waypoints, QMap<QString, QPixmap> &icons);
	void placemark(QList<TrackData> &tracks, QList<Area> &areas,
	  QVector<Waypoint> &waypoints, QMap<QString, QPixmap> &icons);
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
	void style(const QDir &dir, QMap<QString, QPixmap> &icons);
	void iconStyle(const QDir &dir, const QString &id,
	  QMap<QString, QPixmap> &icons);
	void icon(const QDir &dir, const QString &id, QMap<QString, QPixmap> &icons);

	QXmlStreamReader _reader;
};

#endif // KMLPARSER_H
