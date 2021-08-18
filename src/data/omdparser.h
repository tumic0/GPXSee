#ifndef OMDPARSER_H
#define OMDPARSER_H

#include "parser.h"

class OMDParser : public Parser
{
public:
	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &polygons, QVector<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return 0;}

private:
	struct Header
	{
		Header() : date(QDateTime(QDate(1970, 1, 1), QTime(0, 0), Qt::UTC)),
		  elevation(true), hr(true) {}

		QDateTime date;
		bool elevation;
		bool hr;
	};

	struct Sequence
	{
		Sequence() : cnt(0), idx{-1, -1} {}

		unsigned cnt;
		int idx[2];
	};

	bool readHeaderFile(const QString &omdPath, Header &hdr);
	bool readF1(const char *chunk, const Header &hdr, Sequence &seq,
	  SegmentData &segment);
	bool readF2(const char *chunk, const Header &hdr, Sequence &seq,
	  SegmentData &segment);

	QString _errorString;
};

#endif // OMDPARSER_H
