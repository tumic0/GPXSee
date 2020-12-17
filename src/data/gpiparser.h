#ifndef GPIPARSER_H
#define GPIPARSER_H

#include "parser.h"

class DataStream;

class GPIParser : public Parser
{
public:
	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &polygons, QVector<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return 0;}

private:
	bool readFileHeader(DataStream &stream, quint32 &ebs);
	bool readGPIHeader(DataStream &stream);
	bool readData(DataStream &stream, QVector<Waypoint> &waypoints,
	  QList<Area> &polygons, const QString &fileName);

	QString _errorString;
};

#endif // GPIPARSER_H
