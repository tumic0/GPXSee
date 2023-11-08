#ifndef GPSDUMPPARSER_H
#define GPSDUMPPARSER_H

#include "parser.h"

class GPSDumpParser : public Parser
{
public:
	GPSDumpParser() : _errorLine(0) {}

	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &polygons, QVector<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return _errorLine;}

private:
	enum Type {
		Unknown,
		GEO,
		UTM
	};

	int _errorLine;
	QString _errorString;
};

#endif // GPSDUMPPARSER_H
