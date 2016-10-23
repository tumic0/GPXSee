#include "tcxparser.h"


QPointF TCXParser::position()
{
	QPointF pos;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == "LatitudeDegrees")
			pos.setY(_reader.readElementText().toDouble());
		else if (_reader.name() == "LongitudeDegrees")
			pos.setX(_reader.readElementText().toDouble());
		else
			_reader.skipCurrentElement();
	}

	return pos;
}

void TCXParser::trackpointData()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Position")
			_track->last().setCoordinates(position());
		else if (_reader.name() == "AltitudeMeters")
			_track->last().setElevation(_reader.readElementText().toDouble());
		else if (_reader.name() == "Time")
			_track->last().setTimestamp(QDateTime::fromString(
			  _reader.readElementText(), Qt::ISODate));
		else if (_reader.name() == "HeartRateBpm")
			_track->last().setHeartRate(_reader.readElementText().toDouble());
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::routepointData()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Position")
			_route->last().setCoordinates(position());
		else if (_reader.name() == "AltitudeMeters")
			_route->last().setElevation(_reader.readElementText().toDouble());
		else if (_reader.name() == "Time")
			_route->last().setTimestamp(QDateTime::fromString(
			  _reader.readElementText(), Qt::ISODate));
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::waypointData()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Position")
			_waypoints.last().setCoordinates(position());
		else if (_reader.name() == "Name")
			_waypoints.last().setName(_reader.readElementText());
		else if (_reader.name() == "Notes")
			_waypoints.last().setDescription(_reader.readElementText());
		else if (_reader.name() == "AltitudeMeters")
			_waypoints.last().setElevation(
			  _reader.readElementText().toDouble());
		else if (_reader.name() == "Time")
			_waypoints.last().setTimestamp(QDateTime::fromString(
			  _reader.readElementText(), Qt::ISODate));
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::trackpoints()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Trackpoint") {
			_track->append(Trackpoint());
			trackpointData();
		} else
			_reader.skipCurrentElement();
	}
}

void TCXParser::routepoints()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Trackpoint") {
			_route->append(Waypoint());
			routepointData();
		} else
			_reader.skipCurrentElement();
	}
}

void TCXParser::lap()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Track") {
			_tracks.append(QVector<Trackpoint>());
			_track = &_tracks.back();
			trackpoints();
		} else
			_reader.skipCurrentElement();
	}
}

void TCXParser::course()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Track") {
			_routes.append(QVector<Waypoint>());
			_route = &_routes.back();
			routepoints();
		} else if (_reader.name() == "CoursePoint") {
			_waypoints.append(Waypoint());
			waypointData();
		} else
			_reader.skipCurrentElement();
	}
}

void TCXParser::activity()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Lap")
			lap();
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::courses()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Course")
			course();
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::activities()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Activity")
			activity();
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::tcx()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Courses")
			courses();
		else if (_reader.name() == "Activities")
			activities();
		else
			_reader.skipCurrentElement();
	}
}

bool TCXParser::parse()
{
	if (_reader.readNextStartElement()) {
		if (_reader.name() == "TrainingCenterDatabase")
			tcx();
		else
			_reader.raiseError("Not a TCX file.");
	}

	return !_reader.error();
}

bool TCXParser::loadFile(QIODevice *device)
{
	_reader.clear();
	_reader.setDevice(device);

	return parse();
}
