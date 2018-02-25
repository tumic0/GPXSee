#include <QFile>
#include <QXmlStreamReader>
#include "onlinemap.h"
#include "wmtsmap.h"
#include "mapsource.h"


#define ZOOM_MAX       19
#define ZOOM_MIN       0
#define BOUNDS_LEFT    -180
#define BOUNDS_TOP     85.0511
#define BOUNDS_RIGHT   180
#define BOUNDS_BOTTOM  -85.0511

Range MapSource::zooms(QXmlStreamReader &reader)
{
	const QXmlStreamAttributes &attr = reader.attributes();
	int min, max;
	bool res;

	if (attr.hasAttribute("min")) {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
		min = attr.value("min").toString().toInt(&res);
#else // QT_VERSION < 5
		min = attr.value("min").toInt(&res);
#endif // QT_VERSION < 5
		if (!res || (min < ZOOM_MIN || min > ZOOM_MAX)) {
			reader.raiseError("Invalid minimal zoom level");
			return Range();
		}
	} else
		min = ZOOM_MIN;

	if (attr.hasAttribute("max")) {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
		max = attr.value("max").toString().toInt(&res);
#else // QT_VERSION < 5
		max = attr.value("max").toInt(&res);
#endif // QT_VERSION < 5
		if (!res || (max < ZOOM_MIN || max > ZOOM_MAX)) {
			reader.raiseError("Invalid maximal zoom level");
			return Range();
		}
	} else
		max = ZOOM_MAX;

	if (min > max || max < min) {
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
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
		top = attr.value("top").toString().toDouble(&res);
#else // QT_VERSION < 5
		top = attr.value("top").toDouble(&res);
#endif // QT_VERSION < 5
		if (!res || (top < BOUNDS_BOTTOM || top > BOUNDS_TOP)) {
			reader.raiseError("Invalid bounds top value");
			return RectC();
		}
	} else
		top = BOUNDS_TOP;

	if (attr.hasAttribute("bottom")) {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
		bottom = attr.value("bottom").toString().toDouble(&res);
#else // QT_VERSION < 5
		bottom = attr.value("bottom").toDouble(&res);
#endif // QT_VERSION < 5
		if (!res || (bottom < BOUNDS_BOTTOM || bottom > BOUNDS_TOP)) {
			reader.raiseError("Invalid bounds bottom value");
			return RectC();
		}
	} else
		bottom = BOUNDS_BOTTOM;

	if (attr.hasAttribute("left")) {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
		left = attr.value("left").toString().toDouble(&res);
#else // QT_VERSION < 5
		left = attr.value("left").toDouble(&res);
#endif // QT_VERSION < 5
		if (!res || (left < BOUNDS_LEFT || left > BOUNDS_RIGHT)) {
			reader.raiseError("Invalid bounds left value");
			return RectC();
		}
	} else
		left = BOUNDS_LEFT;

	if (attr.hasAttribute("right")) {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
		right = attr.value("right").toString().toDouble(&res);
#else // QT_VERSION < 5
		right = attr.value("right").toDouble(&res);
#endif // QT_VERSION < 5
		if (!res || (right < BOUNDS_LEFT || right > BOUNDS_RIGHT)) {
			reader.raiseError("Invalid bounds right value");
			return RectC();
		}
	} else
		right = BOUNDS_RIGHT;

	if (bottom > top || top < bottom) {
		reader.raiseError("Invalid bottom/top bounds combination");
		return RectC();
	}
	if (left > right || right < left) {
		reader.raiseError("Invalid left/right bounds combination");
		return RectC();
	}

	return RectC(Coordinates(left, top), Coordinates(right, bottom));
}

Map *MapSource::map(QXmlStreamReader &reader)
{
	QString name, url, layer, style, set;
	QString format("image/png");
	bool wmts, rest = false, invertAxis = false;
	Range z(ZOOM_MIN, ZOOM_MAX);
	RectC b(Coordinates(BOUNDS_LEFT, BOUNDS_TOP),
	  Coordinates(BOUNDS_RIGHT, BOUNDS_BOTTOM));

	wmts = (reader.attributes().value("type") == "WMTS") ? true : false;

	while (reader.readNextStartElement()) {
		if (reader.name() == "name")
			name = reader.readElementText();
		else if (reader.name() == "url") {
			rest = (reader.attributes().value("type") == "REST") ? true : false;
			url = reader.readElementText();
		} else if (reader.name() == "zoom") {
			z = zooms(reader);
			reader.skipCurrentElement();
		} else if (reader.name() == "bounds") {
			b = bounds(reader);
			reader.skipCurrentElement();
		} else if (reader.name() == "format")
			format = reader.readElementText();
		else if (reader.name() == "layer")
			layer = reader.readElementText();
		else if (reader.name() == "style")
			style = reader.readElementText();
		else if (reader.name() == "set")
			set = reader.readElementText();
		else if (reader.name() == "axis")
			invertAxis = (reader.readElementText() == "yx") ? true : false;
		else
			reader.skipCurrentElement();
	}

	if (reader.error())
		return 0;
	else if (wmts)
		return new WMTSMap(name, WMTS::Setup(url, layer, set, style, format,
		  rest), invertAxis);
	else
		return new OnlineMap(name, url, z, b);
}

Map *MapSource::loadFile(const QString &path)
{
	QFile file(path);
	QXmlStreamReader reader;
	Map *map = 0;

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		_errorString = file.errorString();
		return map;
	}

	reader.setDevice(&file);

	if (reader.readNextStartElement()) {
		if (reader.name() == "map")
			map = MapSource::map(reader);
		else
			reader.raiseError("Not an online map source file");
	}

	if (!map)
		_errorString = QString("%1: %2").arg(reader.lineNumber())
		  .arg(reader.errorString());
	else if (!map->isValid()) {
		_errorString = map->errorString();
		delete map; map = 0;
	}

	return map;
}
