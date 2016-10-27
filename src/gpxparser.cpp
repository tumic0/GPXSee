#include "gpxparser.h"


qreal GPXParser::number()
{
	bool res;
	qreal ret = _reader.readElementText().toDouble(&res);
	if (!res)
		_reader.raiseError(QString("Invalid %1.").arg(
		  _reader.name().toString()));

	return ret;
}

QDateTime GPXParser::time()
{
	QDateTime d = QDateTime::fromString(_reader.readElementText(),
	  Qt::ISODate);
	if (!d.isValid())
		_reader.raiseError(QString("Invalid %1.").arg(
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
		_reader.raiseError("Invalid longitude.");
		return Coordinates();
	}
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
	lat = attr.value("lat").toString().toDouble(&res);
#else // QT_VERSION < 5
	lat = attr.value("lat").toDouble(&res);
#endif // QT_VERSION < 5
	if (!res || (lat < -90.0 || lat > 90.0)) {
		_reader.raiseError("Invalid latitude.");
		return Coordinates();
	}

	return Coordinates(lon, lat);
}

void GPXParser::handleTrackpointData(DataType type, Trackpoint &trackpoint)
{
	switch (type) {
		case Elevation:
			trackpoint.setElevation(number());
			break;
		case Time:
			trackpoint.setTimestamp(time());
			break;
		case Speed:
			trackpoint.setSpeed(number());
			break;
		case HeartRate:
			trackpoint.setHeartRate(number());
			break;
		case Temperature:
			trackpoint.setTemperature(number());
			break;
		default:
			break;
	}
}

void GPXParser::handleWaypointData(DataType type, Waypoint &waypoint)
{
	switch (type) {
		case Name:
			waypoint.setName(_reader.readElementText());
			break;
		case Description:
			waypoint.setDescription(_reader.readElementText());
			break;
		case Time:
			waypoint.setTimestamp(time());
			break;
		case Elevation:
			waypoint.setElevation(number());
			break;
		default:
			break;
	}
}

void GPXParser::tpExtension(Trackpoint &trackpoint)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "hr")
			handleTrackpointData(HeartRate, trackpoint);
		else if (_reader.name() == "atemp")
			handleTrackpointData(Temperature, trackpoint);
		else
			_reader.skipCurrentElement();
	}
}

void GPXParser::extensions(Trackpoint &trackpoint)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "speed")
			handleTrackpointData(Speed, trackpoint);
		else if (_reader.name() == "hr" || _reader.name() == "heartrate")
			handleTrackpointData(HeartRate, trackpoint);
		else if (_reader.name() == "temp")
			handleTrackpointData(Temperature, trackpoint);
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
			handleTrackpointData(Elevation, trackpoint);
		else if (_reader.name() == "time")
			handleTrackpointData(Time, trackpoint);
		else if (_reader.name() == "geoidheight")
			gh = number();
		else if (_reader.name() == "extensions")
			extensions(trackpoint);
		else
			_reader.skipCurrentElement();
	}

	if (!std::isnan(gh) && !std::isnan(trackpoint.elevation()))
		trackpoint.setElevation(trackpoint.elevation() - gh);
}

void GPXParser::waypointData(Waypoint &waypoint)
{
	qreal gh = NAN;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == "name")
			handleWaypointData(Name, waypoint);
		else if (_reader.name() == "desc")
			handleWaypointData(Description, waypoint);
		else if (_reader.name() == "ele")
			handleWaypointData(Elevation, waypoint);
		else if (_reader.name() == "geoidheight")
			gh = number();
		else if (_reader.name() == "time")
			handleWaypointData(Time, waypoint);
		else
			_reader.skipCurrentElement();
	}

	if (!std::isnan(gh) && !std::isnan(waypoint.elevation()))
		waypoint.setElevation(waypoint.elevation() - gh);
}

void GPXParser::trackpoints()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "trkpt") {
			_track->append(Trackpoint(coordinates()));
			trackpointData(_track->last());
		} else
			_reader.skipCurrentElement();
	}
}

void GPXParser::routepoints()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "rtept") {
			_route->append(Waypoint(coordinates()));
			waypointData(_route->last());
		} else
			_reader.skipCurrentElement();
	}
}

void GPXParser::track()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "trkseg") {
			trackpoints();
		} else
			_reader.skipCurrentElement();
	}
}

void GPXParser::gpx()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "trk") {
			_tracks.append(QVector<Trackpoint>());
			_track = &_tracks.back();
			track();
		} else if (_reader.name() == "rte") {
			_routes.append(QVector<Waypoint>());
			_route = &_routes.back();
			routepoints();
		} else if (_reader.name() == "wpt") {
			_waypoints.append(Waypoint(coordinates()));
			waypointData(_waypoints.last());
		} else
			_reader.skipCurrentElement();
	}
}

bool GPXParser::parse()
{
	if (_reader.readNextStartElement()) {
		if (_reader.name() == "gpx")
			gpx();
		else
			_reader.raiseError("Not a GPX file.");
	}

	return !_reader.error();
}

bool GPXParser::loadFile(QIODevice *device)
{
	_reader.clear();
	_reader.setDevice(device);

	return parse();
}
