#ifndef OV2PARSER_H
#define OV2PARSER_H

#include "parser.h"

class OV2Parser : public Parser
{
	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &polygons, QVector<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return 0;}

private:
	QString _errorString;
};

#endif // OV2PARSER_H
