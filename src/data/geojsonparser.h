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
	  QList<Area> &areas, QVector<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return 0;}

private:
	enum Type {
		Unknown,
		Point,
		LineString,
		MultiPoint,
		Polygon,
		MultiLineString,
		MultiPolygon,
		GeometryCollection,
		Feature,
		FeatureCollection
	};

	Type type(const QJsonObject &json);
	bool point(const QJsonArray &coordinates, Waypoint &waypoint,
	  const QJsonValue &properties = QJsonValue());
	bool multiPoint(const QJsonArray &coordinates,
	  QVector<Waypoint> &waypoints, const QJsonValue &properties = QJsonValue());
	bool lineString(const QJsonArray &coordinates, SegmentData &segment);
	bool lineString(const QJsonArray &coordinates, TrackData &track,
	  const QJsonValue &properties = QJsonValue());
	bool multiLineString(const QJsonArray &coordinates,
	  TrackData &track, const QJsonValue &properties = QJsonValue());
	bool polygon(const QJsonArray &coordinates, ::Polygon &pg);
	bool polygon(const QJsonArray &coordinates, Area &area,
	  const QJsonValue &properties = QJsonValue());
	bool multiPolygon(const QJsonArray &coordinates, Area &area,
	  const QJsonValue &properties = QJsonValue());
	bool geometryCollection(const QJsonObject &json, QList<TrackData> &tracks,
	  QList<Area> &areas, QVector<Waypoint> &waypoints,
	  const QJsonValue &properties = QJsonValue());
	bool feature(const QJsonObject &json, QList<TrackData> &tracks,
	  QList<Area> &areas, QVector<Waypoint> &waypoints);
	bool featureCollection(const QJsonObject &json, QList<TrackData> &tracks,
	  QList<Area> &areas, QVector<Waypoint> &waypoints);

	QString _errorString;
};

#endif // GEOJSONPARSER_H
