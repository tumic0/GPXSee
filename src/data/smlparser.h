#ifndef SMLPARSER_H
#define SMLPARSER_H

#include <QXmlStreamReader>
#include "parser.h"


class SMLParser : public Parser
{
public:
	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &polygons, QVector<Waypoint> &waypoints);
	QString errorString() const {return _reader.errorString();}
	int errorLine() const {return _reader.lineNumber();}

private:
	void sml(QList<TrackData> &tracks);
	void deviceLog(TrackData &track);
	void samples(SegmentData &segment);
	void sample(SegmentData &segment);

	QXmlStreamReader _reader;
};

#endif // SMLPARSER_H
