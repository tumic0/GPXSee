#include <QFile>
#include <QXmlStreamReader>
#include "onlinemap.h"
#include "wmtsmap.h"
#include "wmsmap.h"
#include "mapsource.h"

#define ZOOM_MAX       19
#define ZOOM_MIN       0
#define BOUNDS_LEFT    -180
#define BOUNDS_TOP     85.0511
#define BOUNDS_RIGHT   180
#define BOUNDS_BOTTOM  -85.0511


MapSource::Config::Config() : type(OSM), zooms(ZOOM_MIN, ZOOM_MAX),
  bounds(Coordinates(BOUNDS_LEFT, BOUNDS_TOP), Coordinates(BOUNDS_RIGHT,
  BOUNDS_BOTTOM)), format("image/png"), rest(false) {}


static CoordinateSystem coordinateSystem(QXmlStreamReader &reader)
{
	QXmlStreamAttributes attr = reader.attributes();
	if (attr.value("axis") == "yx")
		return CoordinateSystem::YX;
	else if (attr.value("axis") == "xy")
		return CoordinateSystem::XY;
	else
		return CoordinateSystem::Unknown;
}

Range MapSource::zooms(QXmlStreamReader &reader)
{
	const QXmlStreamAttributes &attr = reader.attributes();
	int min, max;
	bool res;

	if (attr.hasAttribute("min")) {
		min = attr.value("min").toString().toInt(&res);
		if (!res || (min < ZOOM_MIN || min > ZOOM_MAX)) {
			reader.raiseError("Invalid minimal zoom level");
			return Range();
		}
	} else
		min = ZOOM_MIN;

	if (attr.hasAttribute("max")) {
		max = attr.value("max").toString().toInt(&res);
		if (!res || (max < ZOOM_MIN || max > ZOOM_MAX)) {
			reader.raiseError("Invalid maximal zoom level");
			return Range();
		}
	} else
		max = ZOOM_MAX;

	if (min > max) {
		reader.raiseError("Invalid maximal/minimal zoom level combination");
		return Range();
	}

	return Range(min, max);
}

RectC MapSource::bounds(QXmlStreamReader &reader)
{
	const QXmlStreamAttributes &attr = reader.attributes();
	double top, left, bottom, right;
	bool res;

	if (attr.hasAttribute("top")) {
		top = attr.value("top").toString().toDouble(&res);
		if (!res || (top < BOUNDS_BOTTOM || top > BOUNDS_TOP)) {
			reader.raiseError("Invalid bounds top value");
			return RectC();
		}
	} else
		top = BOUNDS_TOP;

	if (attr.hasAttribute("bottom")) {
		bottom = attr.value("bottom").toString().toDouble(&res);
		if (!res || (bottom < BOUNDS_BOTTOM || bottom > BOUNDS_TOP)) {
			reader.raiseError("Invalid bounds bottom value");
			return RectC();
		}
	} else
		bottom = BOUNDS_BOTTOM;

	if (attr.hasAttribute("left")) {
		left = attr.value("left").toString().toDouble(&res);
		if (!res || (left < BOUNDS_LEFT || left > BOUNDS_RIGHT)) {
			reader.raiseError("Invalid bounds left value");
			return RectC();
		}
	} else
		left = BOUNDS_LEFT;

	if (attr.hasAttribute("right")) {
		right = attr.value("right").toString().toDouble(&res);
		if (!res || (right < BOUNDS_LEFT || right > BOUNDS_RIGHT)) {
			reader.raiseError("Invalid bounds right value");
			return RectC();
		}
	} else
		right = BOUNDS_RIGHT;

	if (bottom >= top) {
		reader.raiseError("Invalid bottom/top bounds combination");
		return RectC();
	}
	if (left >= right) {
		reader.raiseError("Invalid left/right bounds combination");
		return RectC();
	}

	return RectC(Coordinates(left, top), Coordinates(right, bottom));
}

void MapSource::map(QXmlStreamReader &reader, Config &config)
{
	const QXmlStreamAttributes &attr = reader.attributes();
	config.type = (attr.value("type") == "WMTS") ? WMTS
	  : (attr.value("type") == "WMS") ? WMS : OSM;

	while (reader.readNextStartElement()) {
		if (reader.name() == "name")
			config.name = reader.readElementText();
		else if (reader.name() == "url") {
			config.rest = (reader.attributes().value("type") == "REST")
			  ? true : false;
			config.url = reader.readElementText();
		} else if (reader.name() == "zoom") {
			config.zooms = zooms(reader);
			reader.skipCurrentElement();
		} else if (reader.name() == "bounds") {
			config.bounds = bounds(reader);
			reader.skipCurrentElement();
		} else if (reader.name() == "format")
			config.format = reader.readElementText();
		else if (reader.name() == "layer")
			config.layer = reader.readElementText();
		else if (reader.name() == "style")
			config.style = reader.readElementText();
		else if (reader.name() == "set") {
			config.coordinateSystem = coordinateSystem(reader);
			config.set = reader.readElementText();
		} else if (reader.name() == "dimension") {
			QXmlStreamAttributes attr = reader.attributes();
			if (!attr.hasAttribute("id"))
				reader.raiseError("Missing dimension id");
			else
				config.dimensions.append(QPair<QString, QString>(
				  attr.value("id").toString(), reader.readElementText()));
		} else if (reader.name() == "crs") {
			config.coordinateSystem = coordinateSystem(reader);
			config.crs = reader.readElementText();
		} else if (reader.name() == "authorization") {
			QXmlStreamAttributes attr = reader.attributes();
			config.authorization = Authorization(
			  attr.value("username").toString(),
			  attr.value("password").toString());
			reader.skipCurrentElement();
		} else
			reader.skipCurrentElement();
	}
}

Map *MapSource::loadMap(const QString &path, QString &errorString)
{
	Config config;
	QFile file(path);


	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		errorString = file.errorString();
		return 0;
	}

	QXmlStreamReader reader(&file);
	if (reader.readNextStartElement()) {
		if (reader.name() == "map")
			map(reader, config);
		else
			reader.raiseError("Not an online map source file");
	}
	if (reader.error()) {
		errorString = QString("%1: %2").arg(reader.lineNumber())
		  .arg(reader.errorString());
		return 0;
	}

	if (config.name.isEmpty()) {
		errorString = "Missing name definition";
		return 0;
	}
	if (config.url.isEmpty()) {
		errorString = "Missing URL definition";
		return 0;
	}
	if (config.type == WMTS || config.type == WMS) {
		if (config.layer.isEmpty()) {
			errorString = "Missing layer definition";
			return 0;
		}
		if (config.format.isEmpty()) {
			errorString = "Missing format definition";
			return 0;
		}
	}
	if (config.type == WMTS) {
		if (config.set.isEmpty()) {
			errorString = "Missing set definiton";
			return 0;
		}
	}
	if (config.type == WMS) {
		if (config.crs.isEmpty()) {
			errorString = "Missing CRS definiton";
			return 0;
		}
	}

	if (config.type == WMTS)
		return new WMTSMap(config.name, WMTS::Setup(config.url, config.layer,
		  config.set, config.style, config.format, config.rest,
		  config.coordinateSystem, config.dimensions, config.authorization));
	else if (config.type == WMS)
		return new WMSMap(config.name, WMS::Setup(config.url, config.layer,
		  config.style, config.format, config.crs, config.coordinateSystem,
		  config.dimensions, config.authorization));
	else
		return new OnlineMap(config.name, config.url, config.zooms,
		  config.bounds, config.authorization);
}
