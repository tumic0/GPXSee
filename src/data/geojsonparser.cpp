#include <QJsonDocument>
#include <QJsonArray>
#include "geojsonparser.h"


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

static Type type(const QJsonObject &json)
{
	QString str(json["type"].toString());

	if (str == "Point")
		return Point;
	else if (str == "MultiPoint")
		return MultiPoint;
	else if (str == "LineString")
		return LineString;
	else if (str == "MultiLineString")
		return MultiLineString;
	else if (str == "Polygon")
		return Polygon;
	else if (str == "MultiPolygon")
		return MultiPolygon;
	else if (str == "GeometryCollection")
		return GeometryCollection;
	else if (str == "Feature")
		return Feature;
	else if (str == "FeatureCollection")
		return FeatureCollection;
	else
		return Unknown;
}

bool GeoJSONParser::point(const QJsonArray &coordinates, Waypoint &waypoint,
  const QJsonObject &properties)
{
	if (coordinates.count() < 2 || !coordinates.at(0).isDouble()
	  || !coordinates.at(1).isDouble()) {
		_errorString = "Invalid Point Coordinates";
		return false;
	}

	waypoint.setCoordinates(Coordinates(coordinates.at(0).toDouble(),
	  coordinates.at(1).toDouble()));
	if (coordinates.count() == 3 && coordinates.at(2).isDouble())
		waypoint.setElevation(coordinates.at(2).toDouble());
	if (properties.contains("title") && properties["title"].isString())
		waypoint.setName(properties["title"].toString());
	if (properties.contains("description")
	  && properties["description"].isString())
		waypoint.setDescription(properties["description"].toString());

	return true;
}

bool GeoJSONParser::multiPoint(const QJsonArray &coordinates,
  QVector<Waypoint> &waypoints, const QJsonObject &properties)
{
	for (int i = 0; i < coordinates.size(); i++) {
		if (!coordinates.at(i).isArray()) {
			_errorString = "Invalid MultiPoint Coordinates";
			return false;
		} else {
			waypoints.resize(waypoints.size() + 1);
			if (!point(coordinates.at(i).toArray(), waypoints.last(), properties))
				return false;
		}
	}

	return true;
}

bool GeoJSONParser::lineString(const QJsonArray &coordinates, TrackData &track,
  const QJsonObject &properties)
{
	if (properties.contains("title") && properties["title"].isString())
		track.setName(properties["title"].toString());
	if (properties.contains("description")
	  && properties["description"].isString())
		track.setDescription(properties["description"].toString());

	for (int i = 0; i < coordinates.size(); i++) {
		QJsonArray point(coordinates.at(i).toArray());
		if (point.count() < 2 || !point.at(0).isDouble()
		  || !point.at(1).isDouble()) {
			_errorString = "Invalid LineString Coordinates";
			return false;
		}

		Trackpoint t(Coordinates(point.at(0).toDouble(),
		  point.at(1).toDouble()));
		if (point.count() == 3 && point.at(2).isDouble())
			t.setElevation(point.at(2).toDouble());
		track.append(t);
	}

	return true;
}

bool GeoJSONParser::multiLineString(const QJsonArray &coordinates,
  QList<TrackData> &tracks, const QJsonObject &properties)
{
	for (int i = 0; i < coordinates.size(); i++) {
		if (!coordinates.at(i).isArray()) {
			_errorString = "Invalid MultiLineString Coordinates";
			return false;
		} else {
			tracks.append(TrackData());
			if (!lineString(coordinates.at(i).toArray(), tracks.last(),
			  properties))
				return false;
		}
	}

	return true;
}

bool GeoJSONParser::geometryCollection(const QJsonObject &json,
  QList<TrackData> &tracks, QVector<Waypoint> &waypoints,
  const QJsonObject &properties)
{
	if (!json.contains("geometries") || !json["geometries"].isArray()) {
		_errorString = "Invalid/missing GeometryCollection geometries array";
		return false;
	}

	QJsonArray geometries(json["geometries"].toArray());
	for (int i = 0; i < geometries.size(); i++) {
		QJsonObject geometry(geometries.at(i).toObject());
		switch (type(geometry)) {
			case Point:
				waypoints.resize(waypoints.size() + 1);
				if (!point(geometry["coordinates"].toArray(), waypoints.last(),
				  properties))
					return false;
				break;
			case MultiPoint:
				if (!multiPoint(geometry["coordinates"].toArray(), waypoints,
				  properties))
					return false;
				break;
			case LineString:
				tracks.append(TrackData());
				if (!lineString(geometry["coordinates"].toArray(),
				  tracks.last(), properties))
					return false;
				break;
			case MultiLineString:
				if (!multiLineString(geometry["coordinates"].toArray(), tracks,
				  properties))
					return false;
				break;
			case Polygon:
			case MultiPolygon:
				break;
			case GeometryCollection:
				if (!geometryCollection(geometry, tracks, waypoints,
				  properties))
					return false;
				break;
			default:
				_errorString = geometry["type"].toString()
				  + ": invalid/missing geometry type";
				return false;
		}
	}

	return true;
}

bool GeoJSONParser::feature(const QJsonObject &json, QList<TrackData> &tracks,
  QVector<Waypoint> &waypoints)
{
	QJsonObject properties(json["properties"].toObject());
	QJsonObject geometry(json["geometry"].toObject());

	switch (type(geometry)) {
		case Point:
			waypoints.resize(waypoints.size() + 1);
			return point(geometry["coordinates"].toArray(), waypoints.last(),
			  properties);
		case MultiPoint:
			return multiPoint(geometry["coordinates"].toArray(), waypoints,
			  properties);
		case LineString:
			tracks.append(TrackData());
			return lineString(geometry["coordinates"].toArray(), tracks.last(),
			  properties);
		case MultiLineString:
			return multiLineString(geometry["coordinates"].toArray(), tracks,
			  properties);
		case GeometryCollection:
			return geometryCollection(geometry, tracks, waypoints);
		case Polygon:
		case MultiPolygon:
			return true;
		default:
			_errorString = geometry["type"].toString()
			  + ": invalid/missing Feature geometry";
			return false;
	}
}

bool GeoJSONParser::featureCollection(const QJsonObject &json,
  QList<TrackData> &tracks, QVector<Waypoint> &waypoints)
{
	if (!json.contains("features") || !json["features"].isArray()) {
		_errorString = "Invalid/missing FeatureCollection features array";
		return false;
	}

	QJsonArray features(json["features"].toArray());
	for (int i = 0; i < features.size(); i++)
		if (!feature(features.at(i).toObject(), tracks, waypoints))
			return false;

	return true;
}


bool GeoJSONParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QVector<Waypoint> &waypoints)
{
	Q_UNUSED(routes);
	QJsonParseError error;
	QJsonDocument doc(QJsonDocument::fromJson(file->readAll(), &error));

	if (doc.isNull()) {
		_errorString = "JSON parse error: " + error.errorString() + " ["
		  + QString::number(error.offset) + "]";
		return false;
	}

	QJsonObject json(doc.object());

	switch (type(json)) {
		case Point:
			waypoints.resize(waypoints.size() + 1);
			return point(json["coordinates"].toArray(), waypoints.last());
		case MultiPoint:
			return multiPoint(json["coordinates"].toArray(), waypoints);
		case LineString:
			tracks.append(TrackData());
			return lineString(json["coordinates"].toArray(), tracks.last());
		case MultiLineString:
			return multiLineString(json["coordinates"].toArray(), tracks);
		case GeometryCollection:
			return geometryCollection(json, tracks, waypoints);
		case Feature:
			return feature(json, tracks, waypoints);
		case FeatureCollection:
			return featureCollection(json, tracks, waypoints);
		case Polygon:
		case MultiPolygon:
			return true;
		case Unknown:
			if (json["type"].toString().isNull())
				_errorString = "Not a GeoJSON file";
			else
				_errorString = json["type"].toString()
				  + ": unknown GeoJSON object";
			return false;
	}

	return true;
}
