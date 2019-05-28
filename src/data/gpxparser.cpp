#include "gpxparser.h"


qreal GPXParser::number()
{
	bool res;
	qreal ret = _reader.readElementText().toDouble(&res);
	if (!res)
		_reader.raiseError(QString("Invalid %1").arg(
		  _reader.name().toString()));

	return ret;
}

QDateTime GPXParser::time()
{
	QDateTime d = QDateTime::fromString(_reader.readElementText(),
	  Qt::ISODate);
	if (!d.isValid())
		_reader.raiseError(QString("Invalid %1").arg(
		  _reader.name().toString()));

	return d;
}

Coordinates GPXParser::coordinates()
{
	bool res;
	qreal lon, lat;
	const QXmlStreamAttributes &attr = _reader.attributes();

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
	lon = attr.value("lon").toString().toDouble(&res);
#else // QT_VERSION < 5
	lon = attr.value("lon").toDouble(&res);
#endif // QT_VERSION < 5
	if (!res || (lon < -180.0 || lon > 180.0)) {
		_reader.raiseError("Invalid longitude");
		return Coordinates();
	}
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
	lat = attr.value("lat").toString().toDouble(&res);
#else // QT_VERSION < 5
	lat = attr.value("lat").toDouble(&res);
#endif // QT_VERSION < 5
	if (!res || (lat < -90.0 || lat > 90.0)) {
		_reader.raiseError("Invalid latitude");
		return Coordinates();
	}

	return Coordinates(lon, lat);
}

void GPXParser::rpExtension(SegmentData *autoRoute)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("rpt"))
			autoRoute->append(Trackpoint(coordinates()));
		_reader.skipCurrentElement();
	}
}

void GPXParser::tpExtension(Trackpoint &trackpoint)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("hr"))
			trackpoint.setHeartRate(number());
		else if (_reader.name() == QLatin1String("atemp"))
			trackpoint.setTemperature(number());
		else if (_reader.name() == QLatin1String("cad"))
			trackpoint.setCadence(number());
		else if (_reader.name() == QLatin1String("speed"))
			trackpoint.setSpeed(number());
		else
			_reader.skipCurrentElement();
	}
}

void GPXParser::rteptExtensions(SegmentData *autoRoute)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("RoutePointExtension"))
			rpExtension(autoRoute);
		else
			_reader.skipCurrentElement();
	}
}

void GPXParser::trkptExtensions(Trackpoint &trackpoint)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("speed"))
			trackpoint.setSpeed(number());
		else if (_reader.name() == QLatin1String("hr")
		  || _reader.name() == QLatin1String("heartrate"))
			trackpoint.setHeartRate(number());
		else if (_reader.name() == QLatin1String("temp"))
			trackpoint.setTemperature(number());
		else if (_reader.name() == QLatin1String("cadence"))
			trackpoint.setCadence(number());
		else if (_reader.name() == QLatin1String("power"))
			trackpoint.setPower(number());
		else if (_reader.name() == QLatin1String("TrackPointExtension"))
			tpExtension(trackpoint);
		else
			_reader.skipCurrentElement();
	}
}

void GPXParser::trackpointData(Trackpoint &trackpoint)
{
	qreal gh = NAN;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("ele"))
			trackpoint.setElevation(number());
		else if (_reader.name() == QLatin1String("time"))
			trackpoint.setTimestamp(time());
		else if (_reader.name() == QLatin1String("geoidheight"))
			gh = number();
		// GPX 1.0
		else if (_reader.name() == QLatin1String("speed"))
			trackpoint.setSpeed(number());
		else if (_reader.name() == QLatin1String("extensions"))
			trkptExtensions(trackpoint);
		else
			_reader.skipCurrentElement();
	}

	if (!std::isnan(gh) && !std::isnan(trackpoint.elevation()))
		trackpoint.setElevation(trackpoint.elevation() - gh);
}

void GPXParser::waypointData(Waypoint &waypoint, SegmentData *autoRoute)
{
	qreal gh = NAN;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("name"))
			waypoint.setName(_reader.readElementText());
		else if (_reader.name() == QLatin1String("desc"))
			waypoint.setDescription(_reader.readElementText());
		else if (_reader.name() == QLatin1String("ele"))
			waypoint.setElevation(number());
		else if (_reader.name() == QLatin1String("geoidheight"))
			gh = number();
		else if (_reader.name() == QLatin1String("time"))
			waypoint.setTimestamp(time());
		else if (autoRoute && _reader.name() == QLatin1String("extensions"))
			rteptExtensions(autoRoute);
		else
			_reader.skipCurrentElement();
	}

	if (!std::isnan(gh) && !std::isnan(waypoint.elevation()))
		waypoint.setElevation(waypoint.elevation() - gh);
}

void GPXParser::trackpoints(SegmentData &segment)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("trkpt")) {
			segment.append(Trackpoint(coordinates()));
			trackpointData(segment.last());
		} else
			_reader.skipCurrentElement();
	}
}

void GPXParser::routepoints(RouteData &route, QList<TrackData> &tracks)
{
	TrackData autoRoute;
	autoRoute.append(SegmentData());
	SegmentData &autoRouteSegment = autoRoute.last();

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("rtept")) {
			route.append(Waypoint(coordinates()));
			waypointData(route.last(), &autoRouteSegment);
		} else if (_reader.name() == QLatin1String("name"))
			route.setName(_reader.readElementText());
		else if (_reader.name() == QLatin1String("desc"))
			route.setDescription(_reader.readElementText());
		else
			_reader.skipCurrentElement();
	}

	if (!autoRouteSegment.isEmpty()) {
		autoRoute.setName(route.name());
		autoRoute.setDescription(route.description());
		tracks.append(autoRoute);
	}
}

void GPXParser::track(TrackData &track)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("trkseg")) {
			track.append(SegmentData());
			trackpoints(track.last());
		} else if (_reader.name() == QLatin1String("name"))
			track.setName(_reader.readElementText());
		else if (_reader.name() == QLatin1String("desc"))
			track.setDescription(_reader.readElementText());
		else
			_reader.skipCurrentElement();
	}
}

void GPXParser::area(Area &area)
{
	area.append(Polygon());
	area.last().append(QVector<Coordinates>());
	QVector<Coordinates> &points = area.last().last();

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("point")) {
			Coordinates c(coordinates());
			_reader.readElementText();
			if (c.isValid())
				points.append(c);
			else
				return;
		} else if (_reader.name() == QLatin1String("name"))
			area.setName(_reader.readElementText());
		else if (_reader.name() == QLatin1String("desc"))
			area.setDescription(_reader.readElementText());
		else
			_reader.skipCurrentElement();
	}
}

void GPXParser::gpxExtensions(QList<Area> &areas)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("area")) {
			areas.append(Area());
			area(areas.last());
		} else
			_reader.skipCurrentElement();
	}
}

void GPXParser::gpx(QList<TrackData> &tracks, QList<RouteData> &routes,
  QList<Area> &areas, QVector<Waypoint> &waypoints)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("trk")) {
			tracks.append(TrackData());
			track(tracks.back());
		} else if (_reader.name() == QLatin1String("rte")) {
			routes.append(RouteData());
			routepoints(routes.back(), tracks);
		} else if (_reader.name() == QLatin1String("wpt")) {
			waypoints.append(Waypoint(coordinates()));
			waypointData(waypoints.last());
		} else if (_reader.name() == QLatin1String("extensions"))
			gpxExtensions(areas);
		else
			_reader.skipCurrentElement();
	}
}

bool GPXParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &areas, QVector<Waypoint> &waypoints)
{
	_reader.clear();
	_reader.setDevice(file);
	_reader.setNamespaceProcessing(false);

	if (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("gpx"))
			gpx(tracks, routes, areas, waypoints);
		else
			_reader.raiseError("Not a GPX file");
	}

	return !_reader.error();
}
