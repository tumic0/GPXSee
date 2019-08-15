#ifndef CUPPARSER_H
#define CUPPARSER_H

#include "parser.h"

class CUPParser : public Parser
{
public:
	CUPParser() : _errorLine(0) {}

	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &polygons, QVector<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return _errorLine;}

private:
	bool waypoint(const QStringList &entry, QVector<Waypoint> &waypoints,
	  QMap<QString, Coordinates> &turnpoints);
	bool task(const QStringList &entry, QList<RouteData> &routes,
	  const QMap<QString, Coordinates> &turnpoints);

	QString _errorString;
	int _errorLine;
};

#endif // CUPPARSER_H
