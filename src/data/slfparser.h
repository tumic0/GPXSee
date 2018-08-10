#ifndef SLFPARSER_H
#define SLFPARSER_H

#include <QXmlStreamReader>
#include "parser.h"

class QDateTime;

class SLFParser : public Parser
{
	bool parse(QFile *file, QList<TrackData> &tracks,
	  QList<RouteData> &routes, QList<Waypoint> &waypoints);
	QString errorString() const {return _reader.errorString();}
	int errorLine() const {return _reader.lineNumber();}

private:
	void generalInformation(QDateTime &date, TrackData &track);
	void activity(TrackData &track);
	void entries(const QDateTime &date, TrackData &track);
	bool data(const QXmlStreamAttributes &attr, const char *name, qreal &val);
	void warning(const char *text) const;

	QXmlStreamReader _reader;
};

#endif // SLFPARSER_H
