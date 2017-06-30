#include "tcxparser.h"


void TCXParser::warning(const char *text) const
{
	const QFile *file = static_cast<QFile *>(_reader.device());
	qWarning("%s:%lld: %s\n", qPrintable(file->fileName()),
	  _reader.lineNumber(), text);
}

qreal TCXParser::number()
{
	bool res;
	qreal ret = _reader.readElementText().toDouble(&res);
	if (!res)
		_reader.raiseError(QString("Invalid %1").arg(
		  _reader.name().toString()));

	return ret;
}

QDateTime TCXParser::time()
{
	QDateTime d = QDateTime::fromString(_reader.readElementText(),
	  Qt::ISODate);
	if (!d.isValid())
		_reader.raiseError(QString("Invalid %1").arg(
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
				_reader.raiseError("Invalid LatitudeDegrees");
			else
				pos.setLat(val);
		} else if (_reader.name() == "LongitudeDegrees") {
			val = _reader.readElementText().toDouble(&res);
			if (!res || (val < -180.0 || val > 180.0))
				_reader.raiseError("Invalid LongitudeDegrees");
			else
				pos.setLon(val);
		} else
			_reader.skipCurrentElement();
	}

	return pos;
}

void TCXParser::extensions(Trackpoint &trackpoint)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "RunCadence")
			trackpoint.setCadence(number());
		else if (_reader.name() == "Watts")
			trackpoint.setPower(number());
		else
			_reader.skipCurrentElement();
	}
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
		else if (_reader.name() == "Cadence")
			trackpoint.setCadence(number());
		else if (_reader.name() == "Extensions")
			extensions(trackpoint);
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

void TCXParser::trackpoints(TrackData &track)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Trackpoint") {
			Trackpoint t;
			trackpointData(t);
			if (t.coordinates().isValid())
				track.append(t);
			else
				warning("Missing Trackpoint coordinates");
		} else
			_reader.skipCurrentElement();
	}
}

void TCXParser::lap(TrackData &track)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Track")
			trackpoints(track);
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::course(TrackData &track)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Track")
			trackpoints(track);
		else if (_reader.name() == "Name")
			track.setName(_reader.readElementText());
		else if (_reader.name() == "Notes")
			track.setDescription(_reader.readElementText());
		else if (_reader.name() == "CoursePoint") {
			Waypoint w;
			waypointData(w);
			if (w.coordinates().isValid())
				_waypoints.append(w);
			else
				warning("Missing Trackpoint coordinates");
		} else
			_reader.skipCurrentElement();
	}
}

void TCXParser::activity(TrackData &track)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Lap")
			lap(track);
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::courses()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Course") {
			_tracks.append(TrackData());
			course(_tracks.back());
		} else
			_reader.skipCurrentElement();
	}
}

void TCXParser::activities()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Activity") {
			_tracks.append(TrackData());
			activity(_tracks.back());
		} else
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
			_reader.raiseError("Not a TCX file");
	}

	return !_reader.error();
}

bool TCXParser::loadFile(QFile *file)
{
	_reader.clear();
	_reader.setDevice(file);

	return parse();
}
