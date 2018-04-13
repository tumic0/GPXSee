#ifndef RTEPARSER_H
#define RTEPARSER_H

#include "parser.h"

class RTEParser : public Parser
{
public:
	RTEParser() : _errorLine(0) {}

	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return _errorLine;}

private:
	QString _errorString;
	int _errorLine;
};

#endif // RTEPARSER_H
