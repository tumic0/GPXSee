#ifndef CSVPARSER_H
#define CSVPARSER_H

#include "parser.h"

class CSVParser : public Parser
{
public:
	CSVParser(QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Waypoint> &waypoints) : Parser(tracks, routes, waypoints)
	  {_errorLine = 0;}
	~CSVParser() {}

	bool loadFile(QFile *file);
	QString errorString() const {return _errorString;}
	int errorLine() const {return _errorLine;}

private:
	QString _errorString;
	int _errorLine;
};

#endif // CSVPARSER_H
