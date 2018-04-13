#ifndef WPTPARSER_H
#define WPTPARSER_H

#include "parser.h"

class WPTParser : public Parser
{
public:
	WPTParser() : _errorLine(0) {}

	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return _errorLine;}

private:
	QString _errorString;
	int _errorLine;
};

#endif // WPTPARSER_H
