#ifndef GPMFPARSER_H
#define GPMFPARSER_H

#include "parser.h"

class GPMFParser : public Parser
{
public:
	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &polygons, QVector<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return 0;}

private:
	bool gpmf(QFile *file, quint64 offset, quint32 size, SegmentData &segment);

	QString _errorString;
};

#endif // GPMFPARSER_H
