#include "smlparser.h"


void SMLParser::sample(SegmentData &segment)
{
	QDateTime timestamp;
	qreal lat = NAN, lon = NAN, altitude = NAN;
	bool ok;

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
		} else
			_reader.skipCurrentElement();
	}

	Trackpoint t(Coordinates((lon * 180) / M_PI, (lat * 180) / M_PI));
	if (!t.coordinates().isValid())
		return;
	t.setTimestamp(timestamp);
	t.setElevation(altitude);

	segment.append(t);
}

void SMLParser::samples(SegmentData &segment)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Sample")) {
			sample(segment);
		} else
			_reader.skipCurrentElement();
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
