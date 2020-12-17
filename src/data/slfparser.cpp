#include "slfparser.h"

void SLFParser::warning(const char *text) const
{
	const QFile *file = static_cast<QFile *>(_reader.device());
	qWarning("%s:%lld: %s", qPrintable(file->fileName()),
	  _reader.lineNumber(), text);
}

bool SLFParser::data(const QXmlStreamAttributes &attr, const char *name,
  qreal &val)
{
	bool res = false;

	if (!attr.hasAttribute(name))
		return false;

	val = attr.value(name).toDouble(&res);
	if (!res)
		_reader.raiseError("Invalid " + QString(name));

	return res;
}

void SLFParser::entries(const QDateTime &date, SegmentData &segment)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Entry")) {
			qreal val, lat, lon;
			QXmlStreamAttributes attr(_reader.attributes());

			if (!data(attr, "longitude", lon) || !data(attr, "latitude", lat)
			  || (lon == 0.0 && lat == 0.0)) {
				if (!_reader.error())
					warning("Missing coordinates");
				_reader.skipCurrentElement();
				continue;
			}

			Trackpoint t(Coordinates(lon, lat));
			if (!t.coordinates().isValid()) {
				_reader.raiseError("Invalid coordinates");
				return;
			}

			if (data(attr, "altitude", val))
				t.setElevation(val / 1000);
			if (data(attr, "cadence", val))
				t.setCadence(val);
			if (data(attr, "heartrate", val))
				t.setHeartRate(val);
			if (data(attr, "power", val))
				t.setPower(val);
			if (data(attr, "speed", val))
				t.setSpeed(val);
			if (data(attr, "trainingTimeAbsolute", val))
				t.setTimestamp(date.addMSecs(val * 10));

			segment.append(t);
		}

		_reader.skipCurrentElement();
	}
}

void SLFParser::generalInformation(QDateTime &date, TrackData &track)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("name"))
			track.setName(_reader.readElementText());
		else if (_reader.name() == QLatin1String("description"))
			track.setDescription(_reader.readElementText());
		else if (_reader.name() == QLatin1String("startDate")) {
			QString ds(_reader.readElementText());
			QLocale locale(QLocale::English);
			date = locale.toDateTime(ds.mid(0, ds.indexOf("GMT"))
			  + ds.right(4), "ddd MMM d HH:mm:ss yyyy");
		} else
			_reader.skipCurrentElement();
	}
}

void SLFParser::activity(TrackData &track)
{
	QDateTime date;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Entries")) {
			track.append(SegmentData());
			entries(date, track.last());
		} else if (_reader.name() == QLatin1String("GeneralInformation"))
			generalInformation(date, track);
		else
			_reader.skipCurrentElement();
	}
}

bool SLFParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons,
  QVector<Waypoint> &waypoints)
{
	Q_UNUSED(waypoints);
	Q_UNUSED(routes);
	Q_UNUSED(polygons);

	_reader.clear();
	_reader.setDevice(file);

	if (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Activity")) {
			tracks.append(TrackData());
			activity(tracks.last());
		} else
			_reader.raiseError("Not a SLF file");
	}

	return !_reader.error();
}
