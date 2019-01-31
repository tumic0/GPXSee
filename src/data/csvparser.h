#ifndef CSVPARSER_H
#define CSVPARSER_H

#include "parser.h"

class CSVParser : public Parser
{
public:
	CSVParser() : _errorLine(0) {}

	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &polygons, QVector<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return _errorLine;}

private:
	QString _errorString;
	int _errorLine;
};

#endif // CSVPARSER_H
