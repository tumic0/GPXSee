#ifndef TCXPARSER_H
#define TCXPARSER_H

#include <QXmlStreamReader>
#include "parser.h"


class TCXParser : public Parser
{
public:
	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &polygons, QVector<Waypoint> &waypoints);
	QString errorString() const {return _reader.errorString();}
	int errorLine() const {return _reader.lineNumber();}

private:
	void tcx(QList<TrackData> &tracks, QVector<Waypoint> &waypoints);
	void courses(QList<TrackData> &tracks, QVector<Waypoint> &waypoints);
	void activities(QList<TrackData> &tracks);
	void multiSportSession(QList<TrackData> &tracks);
	void sport(QList<TrackData> &tracks);
	void course(QVector<Waypoint> &waypoints, TrackData &track);
	void activity(TrackData &track);
	void lap(SegmentData &segment);
	void trackpoints(SegmentData &segment);
	void trackpointData(Trackpoint &trackpoint);
	void waypointData(Waypoint &waypoint);
	void extensions(Trackpoint &trackpoint);
	void heartRateBpm(Trackpoint &trackpoint);
	Coordinates position();
	qreal number();
	QDateTime time();

	void warning(const char *text) const;

	QXmlStreamReader _reader;
};

#endif // TCXPARSER_H
