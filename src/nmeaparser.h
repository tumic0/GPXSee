#ifndef NMEAPARSER_H
#define NMEAPARSER_H

#include <QDate>
#include "parser.h"


class NMEAParser : public Parser
{
public:
	NMEAParser(QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Waypoint> &waypoints) : Parser(tracks, routes, waypoints)
	  {_errorLine = 0; _GGA = false;}
	~NMEAParser() {}

	bool loadFile(QFile *file);
	QString errorString() const {return _errorString;}
	int errorLine() const {return _errorLine;}

private:
	bool readRMC(const char *line, int len);
	bool readGGA(const char *line, int len);
	bool readWPL(const char *line, int len);
	bool readZDA(const char *line, int len);

	int _errorLine;
	QString _errorString;

	QDate _date;
	bool _GGA;
};

#endif // NMEAPARSER_H
