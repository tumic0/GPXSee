#ifndef PLTPARSER_H
#define PLTPARSER_H

#include "parser.h"

class PLTParser : public Parser
{
public:
	PLTParser() : _errorLine(0) {}

	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return _errorLine;}

private:
	QString _errorString;
	int _errorLine;
};

#endif // PLTPARSER_H
