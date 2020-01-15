#ifndef SMLPARSER_H
#define SMLPARSER_H

#include <QXmlStreamReader>
#include <QMap>
#include <QDebug>
#include "parser.h"

class SMLParser : public Parser
{
public:
	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &polygons, QVector<Waypoint> &waypoints);
	QString errorString() const {return _reader.errorString();}
	int errorLine() const {return _reader.lineNumber();}

private:
	struct Sensors
	{
		Sensors()
		  : cadence(NAN), temperature(NAN), hr(NAN), power(NAN), speed(NAN) {}

		qreal cadence, temperature, hr, power, speed;
	};

	void sml(QList<TrackData> &tracks);
	void deviceLog(TrackData &track);
	void samples(SegmentData &segment);
	void sample(SegmentData &segment, QMap<QDateTime, Sensors> &map);

#ifndef QT_NO_DEBUG
	friend QDebug operator<<(QDebug dbg, const Sensors &sensors);
#endif // QT_NO_DEBUG

	QXmlStreamReader _reader;
};

#endif // SMLPARSER_H
