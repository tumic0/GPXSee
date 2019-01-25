#ifndef GEOJSONPARSER_H
#define GEOJSONPARSER_H

#include <QJsonObject>
#include "parser.h"

class QJsonObject;
class QJsonArray;

class GeoJSONParser : public Parser
{
public:
	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QVector<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return 0;}

private:
	bool point(const QJsonArray &coordinates, Waypoint &waypoint,
	  const QJsonObject &properties = QJsonObject());
	bool multiPoint(const QJsonArray &coordinates,
	  QVector<Waypoint> &waypoints, const QJsonObject &properties = QJsonObject());
	bool lineString(const QJsonArray &coordinates, TrackData &track,
	  const QJsonObject &properties = QJsonObject());
	bool multiLineString(const QJsonArray &coordinates,
	  QList<TrackData> &tracks, const QJsonObject &properties = QJsonObject());
	bool geometryCollection(const QJsonObject &json, QList<TrackData> &tracks,
	  QVector<Waypoint> &waypoints, const QJsonObject &properties = QJsonObject());
	bool feature(const QJsonObject &json, QList<TrackData> &tracks,
	  QVector<Waypoint> &waypoints);
	bool featureCollection(const QJsonObject &json, QList<TrackData> &tracks,
	  QVector<Waypoint> &waypoints);

	QString _errorString;
};

#endif // GEOJSONPARSER_H
