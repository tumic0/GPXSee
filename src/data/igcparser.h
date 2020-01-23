#ifndef IGCPARSER_H
#define IGCPARSER_H

#include <QDate>
#include <QTime>
#include "parser.h"


class IGCParser : public Parser
{
public:
	IGCParser() : _errorLine(0) {}

	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &polygons, QVector<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return _errorLine;}

private:
	struct CTX {
		QDate date;
		QTime time;
	};

	bool readHRecord(CTX &ctx, const char *line, int len);
	bool readBRecord(CTX &ctx, const char *line, int len, SegmentData &segment);
	bool readCRecord(const char *line, int len, RouteData &route);

	int _errorLine;
	QString _errorString;
};

#endif // IGCPARSER_H
