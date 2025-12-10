#ifndef MP4PARSER_H
#define MP4PARSER_H

#include "parser.h"

class MP4Parser : public Parser
{
public:
	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &polygons, QVector<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return 0;}

private:
	bool mp4(QFile *file, QVector<quint32> &sizes, QVector<quint64> &chunks,
	  Waypoint &wpt);
	bool gpmf(QFile *file, quint64 offset, quint32 size, SegmentData &segment);

	QString _errorString;
};

#endif // MP4PARSER_H
