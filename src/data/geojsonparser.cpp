#include <QJsonDocument>
#include <QJsonArray>
#include "geojsonparser.h"


GeoJSONParser::Type GeoJSONParser::type(const QJsonObject &json)
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
	if (properties.contains("name") && properties["name"].isString())
		waypoint.setName(properties["name"].toString());
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
			_errorString = "Invalid MultiPoint coordinates";
			return false;
		} else {
			waypoints.resize(waypoints.size() + 1);
			if (!point(coordinates.at(i).toArray(), waypoints.last(), properties))
				return false;
		}
	}

	return true;
}

bool GeoJSONParser::lineString(const QJsonArray &coordinates,
  SegmentData &segment)
{
	for (int i = 0; i < coordinates.size(); i++) {
		QJsonArray point(coordinates.at(i).toArray());
		if (point.count() < 2 || !point.at(0).isDouble()
		  || !point.at(1).isDouble()) {
			_errorString = "Invalid LineString coordinates";
			return false;
		}

		Trackpoint t(Coordinates(point.at(0).toDouble(),
		  point.at(1).toDouble()));
		if (point.count() == 3 && point.at(2).isDouble())
			t.setElevation(point.at(2).toDouble());
		segment.append(t);
	}

	return true;
}

bool GeoJSONParser::lineString(const QJsonArray &coordinates, TrackData &track,
  const QJsonObject &properties)
{
	if (properties.contains("title") && properties["title"].isString())
		track.setName(properties["title"].toString());
	if (properties.contains("name") && properties["name"].isString())
		track.setName(properties["name"].toString());
	if (properties.contains("description")
	  && properties["description"].isString())
		track.setDescription(properties["description"].toString());

	track.append(SegmentData());

	lineString(coordinates, track.last());

	return true;
}

bool GeoJSONParser::multiLineString(const QJsonArray &coordinates,
  TrackData &track, const QJsonObject &properties)
{
	if (properties.contains("title") && properties["title"].isString())
		track.setName(properties["title"].toString());
	if (properties.contains("name") && properties["name"].isString())
		track.setName(properties["name"].toString());
	if (properties.contains("description")
	  && properties["description"].isString())
		track.setDescription(properties["description"].toString());

	for (int i = 0; i < coordinates.size(); i++) {
		if (!coordinates.at(i).isArray()) {
			_errorString = "Invalid MultiLineString coordinates";
			return false;
		} else {
			track.append(SegmentData());
			if (!lineString(coordinates.at(i).toArray(), track.last()))
				return false;
		}
	}

	return true;
}

bool GeoJSONParser::polygon(const QJsonArray &coordinates, ::Polygon &pg)
{
	for (int i = 0; i < coordinates.size(); i++) {
		if (!coordinates.at(i).isArray()) {
			_errorString = "Invalid Polygon linear ring";
			return false;
		}

		const QJsonArray lr(coordinates.at(i).toArray());
		pg.append(QVector<Coordinates>());
		QVector<Coordinates> &data = pg.last();

		for (int j = 0; j < lr.size(); j++) {
			QJsonArray point(lr.at(j).toArray());
			if (point.count() < 2 || !point.at(0).isDouble()
			  || !point.at(1).isDouble()) {
				_errorString = "Invalid Polygon linear ring coordinates";
				return false;
			}
			data.append(Coordinates(point.at(0).toDouble(),
			  point.at(1).toDouble()));
		}
	}

	return true;
}

bool GeoJSONParser::polygon(const QJsonArray &coordinates, Area &area,
  const QJsonObject &properties)
{
	if (properties.contains("title") && properties["title"].isString())
		area.setName(properties["title"].toString());
	if (properties.contains("name") && properties["name"].isString())
		area.setName(properties["name"].toString());
	if (properties.contains("description")
	  && properties["description"].isString())
		area.setDescription(properties["description"].toString());

	area.append(::Polygon());
	return polygon(coordinates, area.last());
}

bool GeoJSONParser::multiPolygon(const QJsonArray &coordinates,
  Area &area, const QJsonObject &properties)
{
	if (properties.contains("title") && properties["title"].isString())
		area.setName(properties["title"].toString());
	if (properties.contains("name") && properties["name"].isString())
		area.setName(properties["name"].toString());
	if (properties.contains("description")
	  && properties["description"].isString())
		area.setDescription(properties["description"].toString());

	for (int i = 0; i < coordinates.size(); i++) {
		if (!coordinates.at(i).isArray()) {
			_errorString = "Invalid MultiPolygon coordinates";
			return false;
		} else {
			area.append(::Polygon());
			if (!polygon(coordinates.at(i).toArray(), area.last()))
				return false;
		}
	}

	return true;
}

bool GeoJSONParser::geometryCollection(const QJsonObject &json,
  QList<TrackData> &tracks, QList<Area> &areas,
  QVector<Waypoint> &waypoints, const QJsonObject &properties)
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
				tracks.append(TrackData());
				if (!multiLineString(geometry["coordinates"].toArray(),
				  tracks.last(), properties))
					return false;
				break;
			case Polygon:
				areas.append(Area());
				if (!polygon(geometry["coordinates"].toArray(), areas.last(),
				  properties))
					return false;
				break;
			case MultiPolygon:
				areas.append(Area());
				if (!multiPolygon(geometry["coordinates"].toArray(),
				  areas.last(), properties))
					return false;
				break;
			case GeometryCollection:
				if (!geometryCollection(geometry, tracks, areas, waypoints,
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
  QList<Area> &areas, QVector<Waypoint> &waypoints)
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
			tracks.append(TrackData());
			return multiLineString(geometry["coordinates"].toArray(),
			  tracks.last(), properties);
		case GeometryCollection:
			return geometryCollection(geometry, tracks, areas, waypoints);
		case Polygon:
			areas.append(Area());
			return polygon(geometry["coordinates"].toArray(), areas.last(),
			  properties);
		case MultiPolygon:
			areas.append(Area());
			return multiPolygon(geometry["coordinates"].toArray(), areas.last(),
			  properties);
		default:
			_errorString = geometry["type"].toString()
			  + ": invalid/missing Feature geometry";
			return false;
	}
}

bool GeoJSONParser::featureCollection(const QJsonObject &json,
  QList<TrackData> &tracks, QList<Area> &areas,
  QVector<Waypoint> &waypoints)
{
	if (!json.contains("features") || !json["features"].isArray()) {
		_errorString = "Invalid/missing FeatureCollection features array";
		return false;
	}

	QJsonArray features(json["features"].toArray());
	for (int i = 0; i < features.size(); i++)
		if (!feature(features.at(i).toObject(), tracks, areas, waypoints))
			return false;

	return true;
}


bool GeoJSONParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &areas, QVector<Waypoint> &waypoints)
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
			tracks.append(TrackData());
			return multiLineString(json["coordinates"].toArray(), tracks.last());
		case GeometryCollection:
			return geometryCollection(json, tracks, areas, waypoints);
		case Feature:
			return feature(json, tracks, areas, waypoints);
		case FeatureCollection:
			return featureCollection(json, tracks, areas, waypoints);
		case Polygon:
			areas.append(Area());
			return polygon(json["coordinates"].toArray(), areas.last());
		case MultiPolygon:
			areas.append(Area());
			return multiPolygon(json["coordinates"].toArray(), areas.last());
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
