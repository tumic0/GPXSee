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

void TCXParser::heartRateBpm(Trackpoint &trackpoint)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Value")
			trackpoint.setHeartRate(number());
		else
			_reader.skipCurrentElement();
	}
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
			heartRateBpm(trackpoint);
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

void TCXParser::course(QList<Waypoint> &waypoints, TrackData &track)
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
				waypoints.append(w);
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

void TCXParser::courses(QList<TrackData> &tracks, QList<Waypoint> &waypoints)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Course") {
			tracks.append(TrackData());
			course(waypoints, tracks.back());
		} else
			_reader.skipCurrentElement();
	}
}

void TCXParser::sport(QList<TrackData> &tracks)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Activity") {
			tracks.append(TrackData());
			activity(tracks.back());
		} else
			_reader.skipCurrentElement();
	}
}

void TCXParser::multiSportSession(QList<TrackData> &tracks)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "FirstSport" || _reader.name() == "NextSport")
			sport(tracks);
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::activities(QList<TrackData> &tracks)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Activity") {
			tracks.append(TrackData());
			activity(tracks.back());
		} else if (_reader.name() == "MultiSportSession")
			multiSportSession(tracks);
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::tcx(QList<TrackData> &tracks, QList<Waypoint> &waypoints)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Courses")
			courses(tracks, waypoints);
		else if (_reader.name() == "Activities")
			activities(tracks);
		else
			_reader.skipCurrentElement();
	}
}

bool TCXParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Waypoint> &waypoints)
{
	Q_UNUSED(routes);

	_reader.clear();
	_reader.setDevice(file);

	if (_reader.readNextStartElement()) {
		if (_reader.name() == "TrainingCenterDatabase")
			tcx(tracks, waypoints);
		else
			_reader.raiseError("Not a TCX file");
	}

	return !_reader.error();
}
