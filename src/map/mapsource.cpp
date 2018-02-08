#include <QFile>
#include <QXmlStreamReader>
#include "onlinemap.h"
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
			reader.raiseError(tr("Invalid minimal zoom level"));
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
			reader.raiseError(tr("Invalid maximal zoom level"));
			return Range();
		}
	} else
		max = ZOOM_MAX;

	if (min > max || max < min) {
		reader.raiseError(tr("Invalid maximal/minimal zoom level combination"));
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
			reader.raiseError(tr("Invalid bounds top value"));
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
			reader.raiseError(tr("Invalid bounds bottom value"));
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
			reader.raiseError(tr("Invalid bounds left value"));
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
			reader.raiseError(tr("Invalid bounds right value"));
			return RectC();
		}
	} else
		right = BOUNDS_RIGHT;

	if (bottom > top || top < bottom) {
		reader.raiseError(tr("Invalid bottom/top bounds combination"));
		return RectC();
	}
	if (left > right || right < left) {
		reader.raiseError(tr("Invalid left/right bounds combination"));
		return RectC();
	}

	return RectC(Coordinates(left, top), Coordinates(right, bottom));
}

void MapSource::map(QXmlStreamReader &reader, Map **map)
{
	QString name, url;
	Range z(ZOOM_MIN, ZOOM_MAX);
	RectC b(Coordinates(BOUNDS_LEFT, BOUNDS_TOP),
	  Coordinates(BOUNDS_RIGHT, BOUNDS_BOTTOM));

	while (reader.readNextStartElement()) {
		if (reader.name() == "name")
			name = reader.readElementText();
		else if (reader.name() == "url")
			url = reader.readElementText();
		else if (reader.name() == "zoom") {
			z = zooms(reader);
			reader.skipCurrentElement();
		} else if (reader.name() == "bounds") {
			b = bounds(reader);
			reader.skipCurrentElement();
		} else
			reader.skipCurrentElement();
	}

	*map = reader.error() ? 0 : new OnlineMap(name, url, z, b);
}

bool MapSource::loadFile(const QString &path, Map **map)
{
	QFile file(path);
	QXmlStreamReader reader;

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		_errorString = file.errorString();
		return false;
	}

	reader.setDevice(&file);

	if (reader.readNextStartElement()) {
		if (reader.name() == "map")
			MapSource::map(reader, map);
		else
			reader.raiseError(tr("Not an online map source file"));
	}

	_errorString = reader.error() ? QString("%1: %2").arg(reader.lineNumber())
	  .arg(reader.errorString()) : QString();

	return !reader.error();
}
