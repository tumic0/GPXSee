#include "smlparser.h"


#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const SMLParser::Sensors &sensors)
{
	dbg.nospace() << "Sensors(" << sensors.cadence << ", "
	  << sensors.temperature << ", " << sensors.hr << "," << sensors.power
	  << ", " << sensors.speed << ")";

	return dbg.space();
}
#endif // QT_NO_DEBUG


void SMLParser::sample(SegmentData &segment, QMap<QDateTime, Sensors> &map)
{
	QDateTime timestamp;
	Sensors sensors;
	qreal lat = NAN, lon = NAN, altitude = NAN;
	bool ok, periodic = false;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Latitude")) {
			lat = _reader.readElementText().toDouble(&ok);
			if (!ok || lat < -90 || lon > 90) {
				_reader.raiseError("Invalid Latitude");
				return;
			}
		} else if (_reader.name() == QLatin1String("Longitude")) {
			lon = _reader.readElementText().toDouble(&ok);
			if (!ok || lat < -180 || lon > 180) {
				_reader.raiseError("Invalid Longitude");
				return;
			}
		} else if (_reader.name() == QLatin1String("UTC")) {
			timestamp = QDateTime::fromString(_reader.readElementText(),
			  Qt::ISODate);
			if (!timestamp.isValid()) {
				_reader.raiseError("Invalid timestamp");
				return;
			}
		} else if (_reader.name() == QLatin1String("GPSAltitude")) {
			altitude = _reader.readElementText().toDouble(&ok);
			if (!ok) {
				_reader.raiseError("Invalid GPS altitude");
				return;
			}
		} else if (_reader.name() == QLatin1String("SampleType")) {
			if (_reader.readElementText() == "periodic")
				periodic = true;
		} else if (_reader.name() == QLatin1String("Cadence")) {
			sensors.cadence = _reader.readElementText().toDouble(&ok);
			if (!ok || sensors.cadence < 0) {
				_reader.raiseError("Invalid Cadence");
				return;
			}
		} else if (_reader.name() == QLatin1String("Temperature")) {
			sensors.temperature = _reader.readElementText().toDouble(&ok);
			// Temperature is in Kelvin units
			if (!ok || sensors.temperature < 0) {
				_reader.raiseError("Invalid Temperature");
				return;
			}
		} else if (_reader.name() == QLatin1String("HR")) {
			sensors.hr = _reader.readElementText().toDouble(&ok);
			if (!ok || sensors.hr < 0) {
				_reader.raiseError("Invalid HR");
				return;
			}
		} else if (_reader.name() == QLatin1String("BikePower")) {
			sensors.power = _reader.readElementText().toDouble(&ok);
			if (!ok || sensors.power < 0) {
				_reader.raiseError("Invalid BikePower");
				return;
			}
		} else if (_reader.name() == QLatin1String("Speed")) {
			sensors.speed = _reader.readElementText().toDouble(&ok);
			if (!ok || sensors.speed < 0) {
				_reader.raiseError("Invalid Speed");
				return;
			}
		} else
			_reader.skipCurrentElement();
	}

	if (periodic && timestamp.isValid())
		map.insert(timestamp, sensors);
	else if (!(std::isnan(lon) || std::isnan(lat))) {
		Trackpoint t(Coordinates(rad2deg(lon), rad2deg(lat)));
		t.setTimestamp(timestamp);
		t.setElevation(altitude);

		segment.append(t);
	}
}

void SMLParser::samples(SegmentData &segment)
{
	QMap<QDateTime, Sensors> sensors;
	QMap<QDateTime, Sensors>::const_iterator it;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Sample")) {
			sample(segment, sensors);
		} else
			_reader.skipCurrentElement();
	}

	for (int i = 0; i < segment.size(); i++) {
		Trackpoint &t = segment[i];
		if ((it = sensors.lowerBound(t.timestamp())) != sensors.constEnd()) {
			t.setCadence(it->cadence * 60);
			t.setTemperature(it->temperature - 273.15);
			t.setHeartRate(it->hr * 60);
			t.setPower(it->power);
			t.setSpeed(it->speed);
		}
	}
}

void SMLParser::deviceLog(TrackData &track)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Samples")) {
			track.append(SegmentData());
			samples(track.last());
		} else
			_reader.skipCurrentElement();
	}
}

void SMLParser::sml(QList<TrackData> &tracks)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("DeviceLog")) {
			tracks.append(TrackData());
			deviceLog(tracks.last());
		} else
			_reader.skipCurrentElement();
	}
}

bool SMLParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons,
  QVector<Waypoint> &waypoints)
{
	Q_UNUSED(routes);
	Q_UNUSED(polygons);
	Q_UNUSED(waypoints);

	_reader.clear();
	_reader.setDevice(file);

	if (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("sml"))
			sml(tracks);
		else
			_reader.raiseError("Not a SML file");
	}

	return !_reader.error();
}
