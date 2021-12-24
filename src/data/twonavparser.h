#ifndef TWONAVPARSER_H
#define TWONAVPARSER_H

#include "parser.h"

class TwoNavParser : public Parser
{
public:
	TwoNavParser() : _errorLine(0) {}

	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &polygons, QVector<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return _errorLine;}

private:
	QString _errorString;
	int _errorLine;
};

#endif // TWONAVPARSER_H
