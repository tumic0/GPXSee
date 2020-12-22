#include <QFile>
#include <QXmlStreamReader>
#include "onlinemap.h"
#include "wmtsmap.h"
#include "wmsmap.h"
#include "osm.h"
#include "invalidmap.h"
#include "mapsource.h"


MapSource::Config::Config() : type(OSM), zooms(OSM::ZOOMS), bounds(OSM::BOUNDS),
  format("image/png"), rest(false), tileRatio(1.0), tileSize(256),
  scalable(false) {}


static CoordinateSystem coordinateSystem(QXmlStreamReader &reader)
{
	QXmlStreamAttributes attr = reader.attributes();
	if (attr.value("axis") == QLatin1String("yx"))
		return CoordinateSystem::YX;
	else if (attr.value("axis") == QLatin1String("xy"))
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
		if (!res || min < 0) {
			reader.raiseError("Invalid minimal zoom level");
			return Range();
		}
	} else
		min = OSM::ZOOMS.min();

	if (attr.hasAttribute("max")) {
		max = attr.value("max").toString().toInt(&res);
		if (!res || max < min) {
			reader.raiseError("Invalid maximal zoom level");
			return Range();
		}
	} else
		max = OSM::ZOOMS.max();

	return Range(min, max);
}

RectC MapSource::bounds(QXmlStreamReader &reader)
{
	const QXmlStreamAttributes &attr = reader.attributes();
	double top, left, bottom, right;
	bool res;

	if (attr.hasAttribute("top")) {
		top = attr.value("top").toString().toDouble(&res);
		if (!res || (top < OSM::BOUNDS.bottom() || top > OSM::BOUNDS.top())) {
			reader.raiseError("Invalid bounds top value");
			return RectC();
		}
	} else
		top = OSM::BOUNDS.top();

	if (attr.hasAttribute("bottom")) {
		bottom = attr.value("bottom").toString().toDouble(&res);
		if (!res || (bottom < OSM::BOUNDS.bottom()
		  || bottom > OSM::BOUNDS.top())) {
			reader.raiseError("Invalid bounds bottom value");
			return RectC();
		}
	} else
		bottom = OSM::BOUNDS.bottom();

	if (attr.hasAttribute("left")) {
		left = attr.value("left").toString().toDouble(&res);
		if (!res || (left < OSM::BOUNDS.left() || left > OSM::BOUNDS.right())) {
			reader.raiseError("Invalid bounds left value");
			return RectC();
		}
	} else
		left = OSM::BOUNDS.left();

	if (attr.hasAttribute("right")) {
		right = attr.value("right").toString().toDouble(&res);
		if (!res || (right < OSM::BOUNDS.left()
		  || right > OSM::BOUNDS.right())) {
			reader.raiseError("Invalid bounds right value");
			return RectC();
		}
	} else
		right = OSM::BOUNDS.right();

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

void MapSource::tile(QXmlStreamReader &reader, Config &config)
{
	QXmlStreamAttributes attr = reader.attributes();
	bool ok;

	if (attr.hasAttribute("size")) {
		int size = attr.value("size").toString().toInt(&ok);
		if (!ok || size < 0) {
			reader.raiseError("Invalid tile size");
			return;
		} else
			config.tileSize = size;
	}
	if (attr.hasAttribute("type")) {
		if (attr.value("type") == QLatin1String("raster"))
			config.scalable = false;
		else if (attr.value("type") == QLatin1String("vector"))
			config.scalable = true;
		else {
			reader.raiseError("Invalid tile type");
			return;
		}
	}
	if (attr.hasAttribute("pixelRatio")) {
		qreal ratio = attr.value("pixelRatio").toString().toDouble(&ok);
		if (!ok || ratio < 0) {
			reader.raiseError("Invalid tile pixelRatio");
			return;
		} else
			config.tileRatio = ratio;
	}
}

void MapSource::map(QXmlStreamReader &reader, Config &config)
{
	const QXmlStreamAttributes &attr = reader.attributes();
	QStringView type = attr.value("type");

	if (type == QLatin1String("WMTS"))
		config.type = WMTS;
	else if (type == QLatin1String("WMS"))
		config.type = WMS;
	else if (type == QLatin1String("TMS"))
		config.type = TMS;
	else if (type == QLatin1String("QuadTiles"))
		config.type = QuadTiles;
	else if (type == QLatin1String("OSM") || type.isEmpty())
		config.type = OSM;
	else {
		reader.raiseError("Invalid map type");
		return;
	}

	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("name"))
			config.name = reader.readElementText();
		else if (reader.name() == QLatin1String("url")) {
			config.rest = (reader.attributes().value("type")
			  == QLatin1String("REST")) ? true : false;
			config.url = reader.readElementText();
		} else if (reader.name() == QLatin1String("zoom")) {
			config.zooms = zooms(reader);
			reader.skipCurrentElement();
		} else if (reader.name() == QLatin1String("bounds")) {
			config.bounds = bounds(reader);
			reader.skipCurrentElement();
		} else if (reader.name() == QLatin1String("format"))
			config.format = reader.readElementText();
		else if (reader.name() == QLatin1String("layer"))
			config.layer = reader.readElementText();
		else if (reader.name() == QLatin1String("style"))
			config.style = reader.readElementText();
		else if (reader.name() == QLatin1String("set")) {
			config.coordinateSystem = coordinateSystem(reader);
			config.set = reader.readElementText();
		} else if (reader.name() == QLatin1String("dimension")) {
			QXmlStreamAttributes attr = reader.attributes();
			if (!attr.hasAttribute("id"))
				reader.raiseError("Missing dimension id");
			else
				config.dimensions.append(KV<QString, QString>
				  (attr.value("id").toString(), reader.readElementText()));
		} else if (reader.name() == QLatin1String("crs")) {
			config.coordinateSystem = coordinateSystem(reader);
			config.crs = reader.readElementText();
		} else if (reader.name() == QLatin1String("authorization")) {
			QXmlStreamAttributes attr = reader.attributes();
			config.authorization = Authorization(
			  attr.value("username").toString(),
			  attr.value("password").toString());
			reader.skipCurrentElement();
		} else if (reader.name() == QLatin1String("tile")) {
			tile(reader, config);
			reader.skipCurrentElement();
		} else
			reader.skipCurrentElement();
	}
}

bool MapSource::isMap(const QString &path)
{
	QFile file(path);

	if (!file.open(QFile::ReadOnly | QFile::Text))
		return false;

	QXmlStreamReader reader(&file);
	if (reader.readNextStartElement() && reader.name() == QLatin1String("map"))
		return true;

	return false;
}

Map *MapSource::loadMap(const QString &path)
{
	Config config;
	QFile file(path);


	if (!file.open(QFile::ReadOnly | QFile::Text))
		return new InvalidMap(path, file.errorString());

	QXmlStreamReader reader(&file);
	if (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("map"))
			map(reader, config);
		else
			reader.raiseError("Not an online map source file");
	}
	if (reader.error())
		return new InvalidMap(path, QString("%1: %2").arg(reader.lineNumber())
		  .arg(reader.errorString()));

	if (config.name.isEmpty())
		return new InvalidMap(path, "Missing name definition");
	if (config.url.isEmpty())
		return new InvalidMap(path, "Missing URL definition");
	if (config.type == WMTS || config.type == WMS) {
		if (config.layer.isEmpty())
			return new InvalidMap(path, "Missing layer definition");
		if (config.format.isEmpty())
			return new InvalidMap(path, "Missing format definition");
	}
	if (config.type == WMTS) {
		if (config.set.isEmpty())
			return new InvalidMap(path, "Missing set definiton");
	}
	if (config.type == WMS) {
		if (config.crs.isEmpty())
			return new InvalidMap(path, "Missing CRS definiton");
	}

	switch (config.type) {
		case WMTS:
			return new WMTSMap(path, config.name, WMTS::Setup(config.url,
			  config.layer, config.set, config.style, config.format, config.rest,
			  config.coordinateSystem, config.dimensions, config.authorization),
			  config.tileRatio);
		case WMS:
			return new WMSMap(path, config.name, WMS::Setup(config.url,
			  config.layer, config.style, config.format, config.crs,
			  config.coordinateSystem, config.dimensions, config.authorization),
			  config.tileSize);
		case TMS:
			return new OnlineMap(path, config.name, config.url, config.zooms,
			  config.bounds, config.tileRatio, config.authorization,
			  config.tileSize, config.scalable, true, false);
		case OSM:
			return new OnlineMap(path, config.name, config.url, config.zooms,
			 config.bounds, config.tileRatio, config.authorization,
			 config.tileSize, config.scalable, false, false);
		case QuadTiles:
			return new OnlineMap(path, config.name, config.url, config.zooms,
			 config.bounds, config.tileRatio, config.authorization,
			 config.tileSize, config.scalable, false, true);
		default:
			return new InvalidMap(path, "Invalid map type");
	}
}
