#ifndef CSVPARSER_H
#define CSVPARSER_H

#include "parser.h"

class CSVParser : public Parser
{
public:
	CSVParser(QList<QVector<Trackpoint> > &tracks,
	  QList<QVector<Waypoint> > &routes, QList<Waypoint> &waypoints)
	  : Parser(tracks, routes, waypoints) {_errorLine = 0;}
	~CSVParser() {}

	bool loadFile(QIODevice *device);
	QString errorString() const {return _errorString;}
	int errorLine() const {return _errorLine;}

private:
	QString _errorString;
	int _errorLine;
};

#endif // CSVPARSER_H
