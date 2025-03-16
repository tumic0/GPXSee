#ifndef VTKPARSER_H
#define VTKPARSER_H

#include "parser.h"

class VTKParser : public Parser
{
public:
	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &polygons, QVector<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return 0;}

private:
	QString _errorString;
};

#endif // VTKPARSER_H
