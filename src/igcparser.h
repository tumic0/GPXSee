#ifndef IGCPARSER_H
#define IGCPARSER_H

#include <QDate>
#include <QTime>
#include "parser.h"


class IGCParser : public Parser
{
public:
	IGCParser(QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Waypoint> &waypoints) : Parser(tracks, routes, waypoints)
	  {_errorLine = 0;}
	~IGCParser() {}

	bool loadFile(QFile *file);
	QString errorString() const {return _errorString;}
	int errorLine() const {return _errorLine;}

private:
	bool readHRecord(const char *line, int len);
	bool readBRecord(const char *line, int len);
	bool readCRecord(const char *line, int len);

	int _errorLine;
	QString _errorString;

	QDate _date;
	QTime _time;
};

#endif // IGCPARSER_H
