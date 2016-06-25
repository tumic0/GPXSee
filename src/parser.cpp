#include "parser.h"


void Parser::handleTrackpointData(TrackpointElement element,
  const QString &value)
{
	switch (element) {
		case Elevation:
			_track->last().elevation = value.toLatin1().toDouble();
			break;
		case Time:
			_track->last().timestamp = QDateTime::fromString(value.toLatin1(),
			  Qt::ISODate);
			break;
		case Geoidheight:
			_track->last().geoidheight = value.toLatin1().toDouble();
			break;
		case Speed:
			_track->last().speed = value.toDouble();
			break;
		case HeartRate:
			_track->last().heartRate = value.toDouble();
			break;
		case Temperature:
			_track->last().temperature = value.toDouble();
			break;
	}
}

void Parser::handleWaypointData(WaypointElement element, const QString &value)
{
	if (element == Name)
		_waypoints.last().setDescription(value);
}

void Parser::handleTrackpointAttributes(const QXmlStreamAttributes &attr)
{
	_track->last().coordinates.setY(attr.value("lat").toLatin1().toDouble());
	_track->last().coordinates.setX(attr.value("lon").toLatin1().toDouble());
}

void Parser::handleWaypointAttributes(const QXmlStreamAttributes &attr)
{
	_waypoints.last().setCoordinates(QPointF(
	  attr.value("lon").toLatin1().toDouble(),
	  attr.value("lat").toLatin1().toDouble()));
}

void Parser::tpExtension()
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

void Parser::extensions()
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

void Parser::trackpointData()
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

void Parser::trackpoints()
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

void Parser::track()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "trkseg") {
			trackpoints();
		} else
			_reader.skipCurrentElement();
	}
}

void Parser::waypointData()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "name")
			handleWaypointData(Name, _reader.readElementText());
		else
			_reader.skipCurrentElement();
	}
}

void Parser::gpx()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "trk") {
			_tracks.append(QVector<Trackpoint>());
			_track = &_tracks.back();
			track();
		} else if (_reader.name() == "wpt") {
			_waypoints.append(Waypoint());
			handleWaypointAttributes(_reader.attributes());
			waypointData();
		} else
			_reader.skipCurrentElement();
	}
}

bool Parser::parse()
{
	if (_reader.readNextStartElement()) {
		if (_reader.name() == "gpx")
			gpx();
		else
			_reader.raiseError("Not a GPX file.");
	}

	return !_reader.error();
}

bool Parser::loadFile(QIODevice *device)
{
	_reader.clear();
	_reader.setDevice(device);

	return parse();
}
