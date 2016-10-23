#include "gpxparser.h"


void GPXParser::handleTrackpointData(DataType type, const QString &value)
{
	switch (type) {
		case Elevation:
			_track->last().setElevation(value.toDouble());
			break;
		case Time:
			_track->last().setTimestamp(QDateTime::fromString(value,
			  Qt::ISODate));
			break;
		case Geoidheight:
			_track->last().setGeoidHeight(value.toDouble());
			break;
		case Speed:
			_track->last().setSpeed(value.toDouble());
			break;
		case HeartRate:
			_track->last().setHeartRate(value.toDouble());
			break;
		case Temperature:
			_track->last().setTemperature(value.toDouble());
			break;
		default:
			break;
	}
}

void GPXParser::handleWaypointData(DataType type, const QString &value)
{
	switch (type) {
		case Name:
			_waypoints.last().setName(value);
			break;
		case Description:
			_waypoints.last().setDescription(value);
			break;
		case Time:
			_waypoints.last().setTimestamp(QDateTime::fromString(value,
			  Qt::ISODate));
			break;
		case Elevation:
			_waypoints.last().setElevation(value.toDouble());
			break;
		case Geoidheight:
			_waypoints.last().setGeoidHeight(value.toDouble());
			break;
		default:
			break;
	}
}

void GPXParser::handleRoutepointData(DataType type, const QString &value)
{
	switch (type) {
		case Name:
			_route->last().setName(value);
			break;
		case Description:
			_route->last().setDescription(value);
			break;
		case Time:
			_route->last().setTimestamp(QDateTime::fromString(value,
			  Qt::ISODate));
			break;
		case Elevation:
			_route->last().setElevation(value.toDouble());
			break;
		case Geoidheight:
			_route->last().setGeoidHeight(value.toDouble());
			break;
		default:
			break;
	}
}

Coordinates GPXParser::coordinates(const QXmlStreamAttributes &attr)
{
	bool res;
	qreal lon, lat;

	lon = attr.value("lon").toDouble(&res);
	if (!res || (lon < -180.0 || lon > 180.0)) {
		_reader.raiseError("Invalid longitude.");
		return Coordinates();
	}
	lat = attr.value("lat").toDouble(&res);
	if (!res || (lat < -90.0 || lat > 90.0)) {
		_reader.raiseError("Invalid latitude.");
		return Coordinates();
	}

	return Coordinates(lon, lat);
}

void GPXParser::handleTrackpointAttributes(const QXmlStreamAttributes &attr)
{
	_track->last().setCoordinates(coordinates(attr));
}

void GPXParser::handleRoutepointAttributes(const QXmlStreamAttributes &attr)
{
	_route->last().setCoordinates(coordinates(attr));
}

void GPXParser::handleWaypointAttributes(const QXmlStreamAttributes &attr)
{
	_waypoints.last().setCoordinates(coordinates(attr));
}

void GPXParser::tpExtension()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "hr")
			handleTrackpointData(HeartRate, _reader.readElementText());
		else if (_reader.name() == "atemp")
			handleTrackpointData(Temperature, _reader.readElementText());
		else
			_reader.skipCurrentElement();
	}
}

void GPXParser::extensions()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "speed")
			handleTrackpointData(Speed, _reader.readElementText());
		else if (_reader.name() == "hr" || _reader.name() == "heartrate")
			handleTrackpointData(HeartRate, _reader.readElementText());
		else if (_reader.name() == "temp")
			handleTrackpointData(Temperature, _reader.readElementText());
		else if (_reader.name() == "TrackPointExtension")
			tpExtension();
		else
			_reader.skipCurrentElement();
	}
}

void GPXParser::trackpointData()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "ele")
			handleTrackpointData(Elevation, _reader.readElementText());
		else if (_reader.name() == "time")
			handleTrackpointData(Time, _reader.readElementText());
		else if (_reader.name() == "geoidheight")
			handleTrackpointData(Geoidheight, _reader.readElementText());
		else if (_reader.name() == "extensions")
			extensions();
		else
			_reader.skipCurrentElement();
	}
}

void GPXParser::routepointData()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "name")
			handleRoutepointData(Name, _reader.readElementText());
		else if (_reader.name() == "desc")
			handleRoutepointData(Description, _reader.readElementText());
		else if (_reader.name() == "ele")
			handleRoutepointData(Elevation, _reader.readElementText());
		else if (_reader.name() == "geoidheight")
			handleRoutepointData(Geoidheight, _reader.readElementText());
		else if (_reader.name() == "time")
			handleRoutepointData(Time, _reader.readElementText());
		else
			_reader.skipCurrentElement();
	}
}

void GPXParser::waypointData()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "name")
			handleWaypointData(Name, _reader.readElementText());
		else if (_reader.name() == "desc")
			handleWaypointData(Description, _reader.readElementText());
		else if (_reader.name() == "ele")
			handleWaypointData(Elevation, _reader.readElementText());
		else if (_reader.name() == "geoidheight")
			handleWaypointData(Geoidheight, _reader.readElementText());
		else if (_reader.name() == "time")
			handleWaypointData(Time, _reader.readElementText());
		else
			_reader.skipCurrentElement();
	}
}

void GPXParser::trackpoints()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "trkpt") {
			_track->append(Trackpoint());
			handleTrackpointAttributes(_reader.attributes());
			trackpointData();
		} else
			_reader.skipCurrentElement();
	}
}

void GPXParser::routepoints()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "rtept") {
			_route->append(Waypoint());
			handleRoutepointAttributes(_reader.attributes());
			routepointData();
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
			_waypoints.append(Waypoint());
			handleWaypointAttributes(_reader.attributes());
			waypointData();
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
