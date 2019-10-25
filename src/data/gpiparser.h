#ifndef GPIPARSER_H
#define GPIPARSER_H

#include "parser.h"

class QDataStream;
class QTextCodec;

class GPIParser : public Parser
{
public:
	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &polygons, QVector<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return 0;}

private:
	bool readFileHeader(QDataStream &stream);
	bool readGPIHeader(QDataStream &stream, QTextCodec *codec);
	bool readEntry(QDataStream &stream, QTextCodec *codec,
	  QVector<Waypoint> &waypoints);
	void readPOIDatabase(QDataStream &stream, QTextCodec *codec,
	  QVector<Waypoint> &waypoints);

	QString _errorString;
};

#endif // GPIPARSER_H
