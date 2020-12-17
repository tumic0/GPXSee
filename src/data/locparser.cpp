#include "locparser.h"

Coordinates LOCParser::coordinates()
{
	bool res;
	const QXmlStreamAttributes &attr = _reader.attributes();

	qreal lon = attr.value("lon").toDouble(&res);
	if (!res || (lon < -180.0 || lon > 180.0)) {
		_reader.raiseError("Invalid longitude");
		return Coordinates();
	}
	qreal lat = attr.value("lat").toDouble(&res);
	if (!res || (lat < -90.0 || lat > 90.0)) {
		_reader.raiseError("Invalid latitude");
		return Coordinates();
	}

	return Coordinates(lon, lat);
}

void LOCParser::waypoint(Waypoint &waypoint)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("name")) {
			const QXmlStreamAttributes &attr = _reader.attributes();
			waypoint.setName(attr.value("id").toString());
			waypoint.setDescription(_reader.readElementText());
		} else if (_reader.name() == QLatin1String("coord")) {
			waypoint.setCoordinates(coordinates());
			_reader.skipCurrentElement();
		} else if (_reader.name() == QLatin1String("link")) {
			const QXmlStreamAttributes &attr = _reader.attributes();
			QString URL(_reader.readElementText());
			if (!URL.isEmpty())
				waypoint.addLink(Link(URL, attr.value("text").toString()));
		} else
			_reader.skipCurrentElement();
	}

	if (waypoint.coordinates().isNull())
		_reader.raiseError("Missing waypoint coordinates");
}

void LOCParser::loc(QVector<Waypoint> &waypoints)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("waypoint")) {
			waypoints.append(Waypoint());
			waypoint(waypoints.last());
		} else
			_reader.skipCurrentElement();
	}
}

bool LOCParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons,
  QVector<Waypoint> &waypoints)
{
	Q_UNUSED(tracks);
	Q_UNUSED(routes);
	Q_UNUSED(polygons);

	_reader.clear();
	_reader.setDevice(file);

	if (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("loc"))
			loc(waypoints);
		else
			_reader.raiseError("Not a LOC file");
	}

	return !_reader.error();
}
