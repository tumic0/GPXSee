#include "tcxparser.h"


qreal TCXParser::number()
{
	bool res;
	qreal ret = _reader.readElementText().toDouble(&res);
	if (!res)
		_reader.raiseError(QString("Invalid %1.").arg(
		  _reader.name().toString()));

	return ret;
}

QDateTime TCXParser::time()
{
	QDateTime d = QDateTime::fromString(_reader.readElementText(),
	  Qt::ISODate);
	if (!d.isValid())
		_reader.raiseError(QString("Invalid %1.").arg(
		  _reader.name().toString()));

	return d;
}

Coordinates TCXParser::position()
{
	Coordinates pos;
	qreal val;
	bool res;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == "LatitudeDegrees") {
			val = _reader.readElementText().toDouble(&res);
			if (!res || (val < -90.0 || val > 90.0))
				_reader.raiseError("Invalid LatitudeDegrees.");
			else
				pos.setLat(val);
		} else if (_reader.name() == "LongitudeDegrees") {
			val = _reader.readElementText().toDouble(&res);
			if (!res || (val < -180.0 || val > 180.0))
				_reader.raiseError("Invalid LongitudeDegrees.");
			else
				pos.setLon(val);
		} else
			_reader.skipCurrentElement();
	}

	return pos;
}

void TCXParser::trackpointData(Trackpoint &trackpoint)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Position")
			trackpoint.setCoordinates(position());
		else if (_reader.name() == "AltitudeMeters")
			trackpoint.setElevation(number());
		else if (_reader.name() == "Time")
			trackpoint.setTimestamp(time());
		else if (_reader.name() == "HeartRateBpm")
			trackpoint.setHeartRate(number());
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::waypointData(Waypoint &waypoint)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Position")
			waypoint.setCoordinates(position());
		else if (_reader.name() == "Name")
			waypoint.setName(_reader.readElementText());
		else if (_reader.name() == "Notes")
			waypoint.setDescription(_reader.readElementText());
		else if (_reader.name() == "AltitudeMeters")
			waypoint.setElevation(number());
		else if (_reader.name() == "Time")
			waypoint.setTimestamp(time());
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::trackpoints(QVector<Trackpoint> &track)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Trackpoint") {
			track.append(Trackpoint());
			trackpointData(track.back());
		} else
			_reader.skipCurrentElement();
	}
}

void TCXParser::lap()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Track") {
			_tracks.append(QVector<Trackpoint>());
			trackpoints(_tracks.back());
		} else
			_reader.skipCurrentElement();
	}
}

void TCXParser::course()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Track") {
			_tracks.append(QVector<Trackpoint>());
			trackpoints(_tracks.back());
		} else if (_reader.name() == "CoursePoint") {
			_waypoints.append(Waypoint());
			waypointData(_waypoints.back());
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
