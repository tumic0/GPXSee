#include <QJsonDocument>
#include <QJsonArray>
#include "map/crs.h"
#include "geojsonparser.h"

#define MARKER_SIZE_MEDIUM 12
#define MARKER_SIZE_SMALL  8
#define MARKER_SIZE_LARGE  16

#define PROJ(object, parent) \
	((object).isNull() ? (parent) : (object))

/*
 * Mapbox Simple Style
 * https://github.com/mapbox/simplestyle-spec
 */
static int markerSize(const QString &str)
{
	if (str == "small")
		return MARKER_SIZE_SMALL;
	else if (str == "medium")
		return MARKER_SIZE_MEDIUM;
	else if (str == "large")
		return MARKER_SIZE_LARGE;
	else
		return -1;
}

static void setAreaProperties(Area &area, const QJsonValue &properties)
{
	QColor strokeColor(0x55, 0x55, 0x55);
	QColor fillColor(0x55, 0x55, 0x55, 0x99);
	double strokeWidth = 2;

	if (properties.isObject()) {
		QJsonObject o(properties.toObject());

		if (o["name"].isString())
			area.setName(o["name"].toString());
		if (o["title"].isString())
			area.setName(o["title"].toString());
		if (o["description"].isString())
			area.setDescription(o["description"].toString());

		if (o["stroke"].isString())
			strokeColor = QColor(o["stroke"].toString());
		if (o["stroke-opacity"].isDouble())
			strokeColor.setAlphaF(o["stroke-opacity"].toDouble());
		if (o["stroke-width"].isDouble())
			strokeWidth = o["stroke-width"].toDouble();
		if (o["fill"].isString())
			fillColor = QColor(o["fill"].toString());
		if (o["fill-opacity"].isDouble())
			fillColor.setAlphaF(o["fill-opacity"].toDouble());
		else
			fillColor.setAlphaF(0.6f);
	}

	area.setStyle(PolygonStyle(fillColor, strokeColor, strokeWidth));
}

static void setTrackProperties(TrackData &track, const QJsonValue &properties)
{
	QColor color(0x55, 0x55, 0x55);
	double width = 2;

	if (properties.isObject()) {
		QJsonObject o(properties.toObject());

		if (o["name"].isString())
			track.setName(o["name"].toString());
		if (o["title"].isString())
			track.setName(o["title"].toString());
		if (o["description"].isString())
			track.setDescription(o["description"].toString());

		if (o["stroke"].isString())
			color = QColor(o["stroke"].toString());
		if (o["stroke-opacity"].isDouble())
			color.setAlphaF(o["stroke-opacity"].toDouble());
		if (o["stroke-width"].isDouble())
			width = o["stroke-width"].toDouble();
	}

	track.setStyle(LineStyle(color, width, Qt::SolidLine));
}

static void setWaypointProperties(Waypoint &waypoint,
  const QJsonValue &properties)
{
	QColor color(0x7e, 0x7e, 0x7e);
	int size = MARKER_SIZE_MEDIUM;

	if (properties.isObject()) {
		QJsonObject o(properties.toObject());

		if (o["name"].isString())
			waypoint.setName(o["name"].toString());
		if (o["title"].isString())
			waypoint.setName(o["title"].toString());
		if (o["description"].isString())
			waypoint.setDescription(o["description"].toString());

		if (o["marker-color"].isString())
			color = QColor(o["marker-color"].toString());
		if (o["marker-symbol"].isString())
			waypoint.setSymbol(o["marker-symbol"].toString());
		if (o["marker-size"].isString())
			size = markerSize(o["marker-size"].toString());
	}

	waypoint.setStyle(PointStyle(color, size));
}

/*
 * Mapbox Coordinate Properties
 * https://github.com/mapbox/geojson-coordinate-properties
 */
static QDateTime timestamp(const QJsonValue &data)
{
	if (data.isString())
		return QDateTime::fromString(data.toString(), Qt::ISODate);
	else if (data.isDouble())
		return QDateTime::fromMSecsSinceEpoch((qint64)data.toDouble());
	else
		return QDateTime();
}

static double hr(const QJsonValue &data)
{
	return data.isDouble() ? data.toDouble() : NAN;
}

static void setSegmentProperties(SegmentData &segment, int segno,
  const QJsonValue &properties)
{
	if (properties.isObject()) {
		QJsonObject o(properties.toObject());

		if (o["coordinateProperties"].isObject()) {
			QJsonObject cp(o["coordinateProperties"].toObject());
			if (cp["times"].isArray()) {
				QJsonArray times(cp["times"].toArray());

				if (segno >= 0) {
					if (times.size() > segno) {
						QJsonArray seg(times.at(segno).toArray());
						if (seg.size() == segment.size()) {
							for (int i = 0; i < seg.size(); i++)
								segment[i].setTimestamp(timestamp(seg.at(i)));
						}
					}
				} else {
					if (times.size() == segment.size()) {
						for (int i = 0; i < times.size(); i++)
							segment[i].setTimestamp(timestamp(times.at(i)));
					}
				}
			}
			if (cp["heart"].isArray()) {
				QJsonArray heart(cp["heart"].toArray());

				if (segno >= 0) {
					if (heart.size() > segno) {
						QJsonArray seg(heart.at(segno).toArray());
						if (seg.size() == segment.size()) {
							for (int i = 0; i < seg.size(); i++)
								segment[i].setHeartRate(hr(seg.at(i)));
						}
					}
				} else {
					if (heart.size() == segment.size()) {
						for (int i = 0; i < heart.size(); i++)
							segment[i].setHeartRate(hr(heart.at(i)));
					}
				}
			}
		}
	}
}


static bool isWS(char c)
{
	return (c == 0x20 || c == 0x09 || c == 0x0A || c == 0x0D) ? true : false;
}

static bool possiblyJSONObject(QFile *file)
{
	char c;

	while (file->getChar(&c)) {
		if (isWS(c))
			continue;
		else if (c == '{')
			return true;
		else
			return false;
	}

	return false;
}

bool GeoJSONParser::a2c(const QJsonArray &data, const Projection &proj,
  Coordinates &c)
{
	c = (data.count() >= 2 && data.at(0).isDouble() && data.at(1).isDouble())
	  ? proj.xy2ll(PointD(data.at(0).toDouble(), data.at(1).toDouble()))
	  : Coordinates();

	if (c.isValid())
		return true;
	else {
		QJsonDocument doc(QJsonDocument::fromVariant(data.toVariantList()));
		_errorString = QString("%1: invalid coordinates")
		  .arg(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
		return false;
	}
}

bool GeoJSONParser::crs(const QJsonObject &object, Projection &proj)
{
	if (!object.contains("crs"))
		return true;

	QJsonObject crsObj(object["crs"].toObject());
	if (crsObj["type"].toString() != "name" || !crsObj.contains("properties")) {
		_errorString = "Invalid crs object";
		return false;
	}
	QString str(crsObj["properties"].toObject()["name"].toString());
	proj = CRS::projection(str);

	if (proj.isValid())
		return true;
	else {
		_errorString = QString("%1: unknown CRS").arg(str);
		return false;
	}
}

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

bool GeoJSONParser::point(const QJsonObject &object, const Projection &parent,
  const QJsonValue &properties, QVector<Waypoint> &waypoints)
{
	if (!object.contains("coordinates")) {
		_errorString = "Missing Point coordinates array";
		return false;
	}
	if (object["coordinates"].isNull())
		return true;
	QJsonArray coordinates(object["coordinates"].toArray());
	if (coordinates.isEmpty())
		return true;
	Projection proj;
	if (!crs(object, proj))
		return false;

	Coordinates c;
	if (!a2c(coordinates, PROJ(proj, parent), c))
		return false;

	Waypoint waypoint(c);
	if (coordinates.count() == 3 && coordinates.at(2).isDouble())
		waypoint.setElevation(coordinates.at(2).toDouble());
	setWaypointProperties(waypoint, properties);
	waypoints.append(waypoint);

	return true;
}

bool GeoJSONParser::multiPoint(const QJsonObject &object,
  const Projection &parent, const QJsonValue &properties,
  QVector<Waypoint> &waypoints)
{
	if (!object.contains("coordinates")) {
		_errorString = "Missing MultiPoint coordinates array";
		return false;
	}
	if (object["coordinates"].isNull())
		return true;
	QJsonArray coordinates(object["coordinates"].toArray());
	Projection proj;
	if (!crs(object, proj))
		return false;
	Coordinates c;

	for (int i = 0; i < coordinates.size(); i++) {
		if (!coordinates.at(i).isArray()) {
			_errorString = "Invalid MultiPoint data";
			return false;
		} else {
			QJsonArray data(coordinates.at(i).toArray());
			if (!a2c(data, PROJ(proj, parent), c))
				return false;

			Waypoint waypoint(c);
			if (data.count() == 3 && data.at(2).isDouble())
				waypoint.setElevation(data.at(2).toDouble());
			setWaypointProperties(waypoint, properties);
			waypoints.append(waypoint);
		}
	}

	return true;
}

bool GeoJSONParser::lineString(const QJsonObject &object,
  const Projection &parent, const QJsonValue &properties,
  QList<TrackData> &tracks)
{
	if (!object.contains("coordinates")) {
		_errorString = "Missing LineString coordinates array";
		return false;
	}
	if (object["coordinates"].isNull())
		return true;
	QJsonArray coordinates(object["coordinates"].toArray());
	if (coordinates.isEmpty())
		return true;
	Projection proj;
	if (!crs(object, proj))
		return false;
	SegmentData segment;
	Coordinates c;

	for (int i = 0; i < coordinates.size(); i++) {
		if (!coordinates.at(i).isArray()) {
			_errorString = "Invalid LineString data";
			return false;
		}

		QJsonArray data(coordinates.at(i).toArray());
		if (!a2c(data, PROJ(proj, parent), c))
			return false;

		Trackpoint t(c);
		if (data.count() == 3 && data.at(2).isDouble())
			t.setElevation(data.at(2).toDouble());
		segment.append(t);
	}

	setSegmentProperties(segment, -1, properties);
	TrackData track(segment);
	setTrackProperties(track, properties);
	tracks.append(track);

	return true;
}

bool GeoJSONParser::multiLineString(const QJsonObject &object,
  const Projection &parent, const QJsonValue &properties,
  QList<TrackData> &tracks)
{
	if (!object.contains("coordinates")) {
		_errorString = "Missing MultiLineString coordinates array";
		return false;
	}
	if (object["coordinates"].isNull())
		return true;
	QJsonArray coordinates(object["coordinates"].toArray());
	if (coordinates.isEmpty())
		return true;
	Projection proj;
	if (!crs(object, proj))
		return false;
	TrackData track;
	Coordinates c;

	for (int i = 0; i < coordinates.size(); i++) {
		if (!coordinates.at(i).isArray()) {
			_errorString = "Invalid MultiLineString data";
			return false;
		} else {
			SegmentData segment;

			QJsonArray ls(coordinates.at(i).toArray());
			for (int j = 0; j < ls.size(); j++) {
				if (!ls.at(j).isArray()) {
					_errorString = "Invalid MultiLineString LineString data";
					return false;
				}

				QJsonArray data(ls.at(j).toArray());
				if (!a2c(data, PROJ(proj, parent), c))
					return false;

				Trackpoint t(c);
				if (data.count() == 3 && data.at(2).isDouble())
					t.setElevation(data.at(2).toDouble());
				segment.append(t);
			}

			setSegmentProperties(segment, track.size(), properties);
			track.append(segment);
		}
	}

	setTrackProperties(track, properties);
	tracks.append(track);

	return true;
}

bool GeoJSONParser::polygon(const QJsonObject &object, const Projection &parent,
  const QJsonValue &properties, QList<Area> &areas)
{
	if (!object.contains("coordinates")) {
		_errorString = "Missing Polygon coordinates array";
		return false;
	}
	if (object["coordinates"].isNull())
		return true;
	QJsonArray coordinates(object["coordinates"].toArray());
	if (coordinates.isEmpty())
		return true;
	Projection proj;
	if (!crs(object, proj))
		return false;
	::Polygon poly;
	Coordinates c;

	for (int i = 0; i < coordinates.size(); i++) {
		if (!coordinates.at(i).isArray()) {
			_errorString = "Invalid Polygon linear ring";
			return false;
		}

		QJsonArray lr(coordinates.at(i).toArray());
		QVector<Coordinates> data;

		for (int j = 0; j < lr.size(); j++) {
			if (!lr.at(j).isArray()) {
				_errorString = "Invalid Polygon linear ring data";
				return false;
			}

			QJsonArray point(lr.at(j).toArray());
			if (!a2c(point, PROJ(proj, parent), c))
				return false;

			data.append(c);
		}

		poly.append(data);
	}

	Area area(poly);
	setAreaProperties(area, properties);
	areas.append(area);

	return true;
}

bool GeoJSONParser::multiPolygon(const QJsonObject &object,
  const Projection &parent, const QJsonValue &properties, QList<Area> &areas)
{
	if (!object.contains("coordinates")) {
		_errorString = "Missing MultiPolygon coordinates array";
		return false;
	}
	if (object["coordinates"].isNull())
		return true;
	QJsonArray coordinates(object["coordinates"].toArray());
	if (coordinates.isEmpty())
		return true;
	Projection proj;
	if (!crs(object, proj))
		return false;
	Area area;
	Coordinates c;

	for (int i = 0; i < coordinates.size(); i++) {
		if (!coordinates.at(i).isArray()) {
			_errorString = "Invalid MultiPolygon data";
			return false;
		} else {
			::Polygon poly;

			QJsonArray polygon(coordinates.at(i).toArray());
			for (int j = 0; j < polygon.size(); j++) {
				if (!polygon.at(j).isArray()) {
					_errorString = "Invalid MultiPolygon linear ring";
					return false;
				}

				QJsonArray lr(polygon.at(j).toArray());
				QVector<Coordinates> data;

				for (int k = 0; k < lr.size(); k++) {
					if (!lr.at(k).isArray()) {
						_errorString = "Invalid MultiPolygon linear ring data";
						return false;
					}

					QJsonArray point(lr.at(k).toArray());
					if (!a2c(point, PROJ(proj, parent), c))
						return false;

					data.append(c);
				}

				poly.append(data);
			}

			area.append(poly);
		}
	}

	setAreaProperties(area, properties);
	areas.append(area);

	return true;
}

bool GeoJSONParser::geometryCollection(const QJsonObject &object,
  const Projection &parent, const QJsonValue &properties,
  QList<TrackData> &tracks, QList<Area> &areas, QVector<Waypoint> &waypoints)
{
	if (!object["geometries"].isArray()) {
		_errorString = "Invalid/missing GeometryCollection geometries array";
		return false;
	}

	QJsonArray geometries(object["geometries"].toArray());
	Projection proj;
	if (!crs(object, proj))
		return false;

	for (int i = 0; i < geometries.size(); i++) {
		QJsonObject geometry(geometries.at(i).toObject());

		switch (type(geometry)) {
			case Point:
				if (!point(geometry, PROJ(proj, parent), properties,
				  waypoints))
					return false;
				break;
			case MultiPoint:
				if (!multiPoint(geometry, PROJ(proj, parent), properties,
				  waypoints))
					return false;
				break;
			case LineString:
				if (!lineString(geometry, PROJ(proj, parent), properties,
				  tracks))
					return false;
				break;
			case MultiLineString:
				if (!multiLineString(geometry, PROJ(proj, parent), properties,
				  tracks))
					return false;
				break;
			case Polygon:
				if (!polygon(geometry, PROJ(proj, parent), properties, areas))
					return false;
				break;
			case MultiPolygon:
				if (!multiPolygon(geometry, PROJ(proj, parent), properties,
				  areas))
					return false;
				break;
			case GeometryCollection:
				if (!geometryCollection(geometry, PROJ(proj, parent),
				  properties, tracks, areas, waypoints))
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

bool GeoJSONParser::feature(const QJsonObject &object, const Projection &parent,
  QList<TrackData> &tracks, QList<Area> &areas, QVector<Waypoint> &waypoints)
{
	if (object["geometry"].isNull())
		return true;
	if (!object["geometry"].isObject()) {
		_errorString = "Invalid/missing Feature geometry object";
		return false;
	}

	QJsonValue properties(object["properties"]);
	QJsonObject geometry(object["geometry"].toObject());
	Projection proj;
	if (!crs(object, proj))
		return false;

	switch (type(geometry)) {
		case Point:
			return point(geometry, PROJ(proj, parent), properties, waypoints);
		case MultiPoint:
			return multiPoint(geometry, PROJ(proj, parent), properties,
			  waypoints);
		case LineString:
			return lineString(geometry, PROJ(proj, parent), properties, tracks);
		case MultiLineString:
			return multiLineString(geometry, PROJ(proj, parent), properties,
			  tracks);
		case GeometryCollection:
			return geometryCollection(geometry, PROJ(proj, parent), properties,
			  tracks, areas, waypoints);
		case Polygon:
			return polygon(geometry, PROJ(proj, parent), properties, areas);
		case MultiPolygon:
			return multiPolygon(geometry, PROJ(proj, parent), properties, areas);
		default:
			_errorString = geometry["type"].toString()
			  + ": invalid/missing Feature geometry";
			return false;
	}
}

bool GeoJSONParser::featureCollection(const QJsonObject &object,
  const Projection &parent, QList<TrackData> &tracks, QList<Area> &areas,
  QVector<Waypoint> &waypoints)
{
	if (!object["features"].isArray()) {
		_errorString = "Invalid/missing FeatureCollection features array";
		return false;
	}

	QJsonArray features(object["features"].toArray());
	Projection proj;
	if (!crs(object, proj))
		return false;

	for (int i = 0; i < features.size(); i++)
		if (!feature(features.at(i).toObject(), PROJ(proj, parent), tracks,
		  areas, waypoints))
			return false;

	return true;
}


bool GeoJSONParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &areas, QVector<Waypoint> &waypoints)
{
	Q_UNUSED(routes);

	if (!possiblyJSONObject(file)) {
		_errorString = "Not a GeoJSON file";
		return false;
	} else
		file->reset();

	QJsonParseError error;
	QJsonDocument doc(QJsonDocument::fromJson(file->readAll(), &error));

	if (doc.isNull()) {
		_errorString = QString("JSON parse error on offset %1: %2")
		  .arg(QString::number(error.offset), error.errorString());
		return false;
	}

	QJsonObject object(doc.object());
	Projection proj(GCS::WGS84());

	switch (type(object)) {
		case Point:
			return point(object, proj, QJsonValue(), waypoints);
		case MultiPoint:
			return multiPoint(object, proj, QJsonValue(), waypoints);
		case LineString:
			return lineString(object, proj, QJsonValue(), tracks);
		case MultiLineString:
			return multiLineString(object, proj, QJsonValue(), tracks);
		case GeometryCollection:
			return geometryCollection(object, proj, QJsonValue(), tracks, areas,
			  waypoints);
		case Feature:
			return feature(object, proj, tracks, areas, waypoints);
		case FeatureCollection:
			return featureCollection(object, proj, tracks, areas, waypoints);
		case Polygon:
			return polygon(object, proj, QJsonValue(), areas);
		case MultiPolygon:
			return multiPolygon(object, proj, QJsonValue(), areas);
		case Unknown:
			if (object["type"].toString().isNull())
				_errorString = "Not a GeoJSON file";
			else
				_errorString = object["type"].toString()
				  + ": unknown GeoJSON object";
			return false;
	}

	return true;
}
