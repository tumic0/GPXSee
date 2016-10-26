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

void GPXParser::handleTrackpointData(DataType type)
{
	switch (type) {
		case Elevation:
			_track->last().setElevation(number());
			break;
		case Time:
			_track->last().setTimestamp(time());
			break;
		case Speed:
			_track->last().setSpeed(number());
			break;
		case HeartRate:
			_track->last().setHeartRate(number());
			break;
		case Temperature:
			_track->last().setTemperature(number());
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

void GPXParser::tpExtension()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "hr")
			handleTrackpointData(HeartRate);
		else if (_reader.name() == "atemp")
			handleTrackpointData(Temperature);
		else
			_reader.skipCurrentElement();
	}
}

void GPXParser::extensions()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "speed")
			handleTrackpointData(Speed);
		else if (_reader.name() == "hr" || _reader.name() == "heartrate")
			handleTrackpointData(HeartRate);
		else if (_reader.name() == "temp")
			handleTrackpointData(Temperature);
		else if (_reader.name() == "TrackPointExtension")
			tpExtension();
		else
			_reader.skipCurrentElement();
	}
}

void GPXParser::trackpointData()
{
	qreal gh = NAN;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == "ele")
			handleTrackpointData(Elevation);
		else if (_reader.name() == "time")
			handleTrackpointData(Time);
		else if (_reader.name() == "geoidheight")
			gh = number();
		else if (_reader.name() == "extensions")
			extensions();
		else
			_reader.skipCurrentElement();
	}

	Trackpoint &t = _track->last();
	if (!std::isnan(gh) && !std::isnan(t.elevation()))
		t.setElevation(t.elevation() - gh);
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
			trackpointData();
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
