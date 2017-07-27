#ifndef NMEAPARSER_H
#define NMEAPARSER_H

#include <QDate>
#include "parser.h"


class NMEAParser : public Parser
{
public:
	NMEAParser() : _errorLine(0), _GGA(false) {}
	~NMEAParser() {}

	bool parse(QFile *file, QList<TrackData> &tracks,
	  QList<RouteData> &routes, QList<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return _errorLine;}

private:
	bool readEW(const char *data, int len, qreal &lon);
	bool readLon(const char *data, int len, qreal &lon);
	bool readNS(const char *data, int len, qreal &lat);
	bool readLat(const char *data, int len, qreal &lat);
	bool readDate(const char *data, int len, QDate &date);
	bool readTime(const char *data, int len, QTime &time);
	bool readAltitude(const char *data, int len, qreal &ele);
	bool readGeoidHeight(const char *data, int len, qreal &gh);

	bool readRMC(TrackData &track, const char *line, int len);
	bool readGGA(TrackData &track, const char *line, int len);
	bool readWPL(QList<Waypoint> &waypoints, const char *line, int len);
	bool readZDA(const char *line, int len);

	int _errorLine;
	QString _errorString;

	QDate _date;
	QTime _time;
	bool _GGA;
};

#endif // NMEAPARSER_H
