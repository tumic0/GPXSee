#include "tcxparser.h"


Coordinates TCXParser::position()
{
	Coordinates pos;
	qreal val;
	bool res;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == "LatitudeDegrees") {
			val = _reader.readElementText().toDouble(&res);
			if (!res || (val < -90.0 || val > 90.0))
				_reader.raiseError("Invalid latitude.");
			else
				pos.setLat(val);
		} else if (_reader.name() == "LongitudeDegrees") {
			val = _reader.readElementText().toDouble(&res);
			if (!res || (val < -180.0 || val > 180.0))
				_reader.raiseError("Invalid longitude.");
			else
				pos.setLon(val);
		} else
			_reader.skipCurrentElement();
	}

	return pos;
}

void TCXParser::trackpointData(Trackpoint &t)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Position")
			t.setCoordinates(position());
		else if (_reader.name() == "AltitudeMeters")
			t.setElevation(_reader.readElementText().toDouble());
		else if (_reader.name() == "Time")
			t.setTimestamp(QDateTime::fromString(_reader.readElementText(),
			  Qt::ISODate));
		else if (_reader.name() == "HeartRateBpm")
			t.setHeartRate(_reader.readElementText().toDouble());
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::routepointData(Waypoint &w)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Position")
			w.setCoordinates(position());
		else if (_reader.name() == "AltitudeMeters")
			w.setElevation(_reader.readElementText().toDouble());
		else if (_reader.name() == "Time")
			w.setTimestamp(QDateTime::fromString(_reader.readElementText(),
			  Qt::ISODate));
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::waypointData(Waypoint &w)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Position")
			w.setCoordinates(position());
		else if (_reader.name() == "Name")
			w.setName(_reader.readElementText());
		else if (_reader.name() == "Notes")
			w.setDescription(_reader.readElementText());
		else if (_reader.name() == "AltitudeMeters")
			w.setElevation(_reader.readElementText().toDouble());
		else if (_reader.name() == "Time")
			w.setTimestamp(QDateTime::fromString(_reader.readElementText(),
			  Qt::ISODate));
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::trackpoints()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Trackpoint") {
			Trackpoint t;
			trackpointData(t);
			if (t.coordinates().isValid())
				_track->append(t);
		} else
			_reader.skipCurrentElement();
	}
}

void TCXParser::routepoints()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Trackpoint") {
			Waypoint w;
			routepointData(w);
			if (w.coordinates().isValid())
				_route->append(w);
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
			Waypoint w;
			waypointData(w);
			if (w.coordinates().isValid())
				_waypoints.append(w);
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
