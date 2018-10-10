#include "tcxparser.h"


void TCXParser::warning(const char *text) const
{
	const QFile *file = static_cast<QFile *>(_reader.device());
	qWarning("%s:%lld: %s", qPrintable(file->fileName()),
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
		if (_reader.name() == QLatin1String("LatitudeDegrees")) {
			val = _reader.readElementText().toDouble(&res);
			if (!res || (val < -90.0 || val > 90.0))
				_reader.raiseError("Invalid LatitudeDegrees");
			else
				pos.setLat(val);
		} else if (_reader.name() == QLatin1String("LongitudeDegrees")) {
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
		if (_reader.name() == QLatin1String("Value"))
			trackpoint.setHeartRate(number());
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::extensions(Trackpoint &trackpoint)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("RunCadence"))
			trackpoint.setCadence(number());
		else if (_reader.name() == QLatin1String("Watts"))
			trackpoint.setPower(number());
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::trackpointData(Trackpoint &trackpoint)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Position"))
			trackpoint.setCoordinates(position());
		else if (_reader.name() == QLatin1String("AltitudeMeters"))
			trackpoint.setElevation(number());
		else if (_reader.name() == QLatin1String("Time"))
			trackpoint.setTimestamp(time());
		else if (_reader.name() == QLatin1String("HeartRateBpm"))
			heartRateBpm(trackpoint);
		else if (_reader.name() == QLatin1String("Cadence"))
			trackpoint.setCadence(number());
		else if (_reader.name() == QLatin1String("Extensions"))
			extensions(trackpoint);
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::waypointData(Waypoint &waypoint)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Position"))
			waypoint.setCoordinates(position());
		else if (_reader.name() == QLatin1String("Name"))
			waypoint.setName(_reader.readElementText());
		else if (_reader.name() == QLatin1String("Notes"))
			waypoint.setDescription(_reader.readElementText());
		else if (_reader.name() == QLatin1String("AltitudeMeters"))
			waypoint.setElevation(number());
		else if (_reader.name() == QLatin1String("Time"))
			waypoint.setTimestamp(time());
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::trackpoints(TrackData &track)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Trackpoint")) {
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
		if (_reader.name() == QLatin1String("Track"))
			trackpoints(track);
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::course(QList<Waypoint> &waypoints, TrackData &track)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Track"))
			trackpoints(track);
		else if (_reader.name() == QLatin1String("Name"))
			track.setName(_reader.readElementText());
		else if (_reader.name() == QLatin1String("Notes"))
			track.setDescription(_reader.readElementText());
		else if (_reader.name() == QLatin1String("CoursePoint")) {
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
		if (_reader.name() == QLatin1String("Lap"))
			lap(track);
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::courses(QList<TrackData> &tracks, QList<Waypoint> &waypoints)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Course")) {
			tracks.append(TrackData());
			course(waypoints, tracks.back());
		} else
			_reader.skipCurrentElement();
	}
}

void TCXParser::sport(QList<TrackData> &tracks)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Activity")) {
			tracks.append(TrackData());
			activity(tracks.back());
		} else
			_reader.skipCurrentElement();
	}
}

void TCXParser::multiSportSession(QList<TrackData> &tracks)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("FirstSport")
		  || _reader.name() == QLatin1String("NextSport"))
			sport(tracks);
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::activities(QList<TrackData> &tracks)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Activity")) {
			tracks.append(TrackData());
			activity(tracks.back());
		} else if (_reader.name() == QLatin1String("MultiSportSession"))
			multiSportSession(tracks);
		else
			_reader.skipCurrentElement();
	}
}

void TCXParser::tcx(QList<TrackData> &tracks, QList<Waypoint> &waypoints)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Courses"))
			courses(tracks, waypoints);
		else if (_reader.name() == QLatin1String("Activities"))
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
		if (_reader.name() == QLatin1String("TrainingCenterDatabase"))
			tcx(tracks, waypoints);
		else
			_reader.raiseError("Not a TCX file");
	}

	return !_reader.error();
}
