#ifndef VKXPARSER_H
#define VKXPARSER_H

#include "parser.h"

class QDataStream;

class VKXParser : public Parser
{
public:
	VKXParser()
	{
		static_assert(sizeof(float) == 4, "Invalid float size");
	}

	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &polygons, QVector<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return 0;}

private:
	bool skip(QDataStream &stream, quint8 key, int len);

	QString _errorString;
};

#endif // VKXPARSER_H
