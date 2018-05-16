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

void GPXParser::rpExtension(TrackData *autoRoute)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "rpt")
			autoRoute->append(Trackpoint(coordinates()));
		_reader.skipCurrentElement();
	}
}

void GPXParser::tpExtension(Trackpoint &trackpoint)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "hr")
			trackpoint.setHeartRate(number());
		else if (_reader.name() == "atemp")
			trackpoint.setTemperature(number());
		else
			_reader.skipCurrentElement();
	}
}

void GPXParser::rteptExtensions(TrackData *autoRoute)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "RoutePointExtension")
			rpExtension(autoRoute);
		else
			_reader.skipCurrentElement();
	}
}

void GPXParser::trkptExtensions(Trackpoint &trackpoint)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "speed")
			trackpoint.setSpeed(number());
		else if (_reader.name() == "hr" || _reader.name() == "heartrate")
			trackpoint.setHeartRate(number());
		else if (_reader.name() == "temp")
			trackpoint.setTemperature(number());
		else if (_reader.name() == "cadence")
			trackpoint.setCadence(number());
		else if (_reader.name() == "power")
			trackpoint.setPower(number());
		else if (_reader.name() == "TrackPointExtension")
			tpExtension(trackpoint);
		else
			_reader.skipCurrentElement();
	}
}

void GPXParser::trackpointData(Trackpoint &trackpoint)
{
	qreal gh = NAN;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == "ele")
			trackpoint.setElevation(number());
		else if (_reader.name() == "time")
			trackpoint.setTimestamp(time());
		else if (_reader.name() == "geoidheight")
			gh = number();
		else if (_reader.name() == "extensions")
			trkptExtensions(trackpoint);
		else
			_reader.skipCurrentElement();
	}

	if (!std::isnan(gh) && !std::isnan(trackpoint.elevation()))
		trackpoint.setElevation(trackpoint.elevation() - gh);
}

void GPXParser::waypointData(Waypoint &waypoint, TrackData *autoRoute)
{
	qreal gh = NAN;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == "name")
			waypoint.setName(_reader.readElementText());
		else if (_reader.name() == "desc")
			waypoint.setDescription(_reader.readElementText());
		else if (_reader.name() == "ele")
			waypoint.setElevation(number());
		else if (_reader.name() == "geoidheight")
			gh = number();
		else if (_reader.name() == "time")
			waypoint.setTimestamp(time());
		else if (autoRoute && _reader.name() == "extensions")
			rteptExtensions(autoRoute);
		else
			_reader.skipCurrentElement();
	}

	if (!std::isnan(gh) && !std::isnan(waypoint.elevation()))
		waypoint.setElevation(waypoint.elevation() - gh);
}

void GPXParser::trackpoints(TrackData &track)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "trkpt") {
			track.append(Trackpoint(coordinates()));
			trackpointData(track.last());
		} else
			_reader.skipCurrentElement();
	}
}

void GPXParser::routepoints(RouteData &route, QList<TrackData> &tracks)
{
	TrackData autoRoute;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == "rtept") {
			route.append(Waypoint(coordinates()));
			waypointData(route.last(), &autoRoute);
		} else if (_reader.name() == "name")
			route.setName(_reader.readElementText());
		else if (_reader.name() == "desc")
			route.setDescription(_reader.readElementText());
		else
			_reader.skipCurrentElement();
	}

	if (!autoRoute.isEmpty()) {
		autoRoute.setName(route.name());
		autoRoute.setDescription(route.description());
		tracks.append(autoRoute);
	}
}

void GPXParser::track(TrackData &track)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "trkseg")
			trackpoints(track);
		else if (_reader.name() == "name")
			track.setName(_reader.readElementText());
		else if (_reader.name() == "desc")
			track.setDescription(_reader.readElementText());
		else
			_reader.skipCurrentElement();
	}
}

void GPXParser::gpx(QList<TrackData> &tracks, QList<RouteData> &routes,
  QList<Waypoint> &waypoints)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "trk") {
			tracks.append(TrackData());
			track(tracks.back());
		} else if (_reader.name() == "rte") {
			routes.append(RouteData());
			routepoints(routes.back(), tracks);
		} else if (_reader.name() == "wpt") {
			waypoints.append(Waypoint(coordinates()));
			waypointData(waypoints.last());
		} else
			_reader.skipCurrentElement();
	}
}

bool GPXParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Waypoint> &waypoints)
{
	_reader.clear();
	_reader.setDevice(file);
	_reader.setNamespaceProcessing(false);

	if (_reader.readNextStartElement()) {
		if (_reader.name() == "gpx")
			gpx(tracks, routes, waypoints);
		else
			_reader.raiseError("Not a GPX file");
	}

	return !_reader.error();
}
