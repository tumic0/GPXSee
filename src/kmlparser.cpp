#include "kmlparser.h"


qreal KMLParser::number()
{
	bool res;
	qreal ret = _reader.readElementText().toDouble(&res);
	if (!res)
		_reader.raiseError(QString("Invalid %1").arg(
		  _reader.name().toString()));

	return ret;
}

QDateTime KMLParser::time()
{
	QDateTime d = QDateTime::fromString(_reader.readElementText(),
	  Qt::ISODate);
	if (!d.isValid())
		_reader.raiseError(QString("Invalid %1").arg(
		  _reader.name().toString()));

	return d;
}

bool KMLParser::coord(Trackpoint &trackpoint)
{
	QString data = _reader.readElementText();
	const QChar *sp, *ep, *cp, *vp;
	int c = 0;
	qreal val[3];
	bool res;


	sp = data.constData();
	ep = sp + data.size();

	for (cp = sp; cp < ep; cp++)
		if (!cp->isSpace())
			break;

	for (vp = cp; cp <= ep; cp++) {
		if (cp->isSpace() || cp->isNull()) {
			if (c > 2)
				return false;

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
			val[c] = QString(vp, cp - vp).toDouble(&res);
#else // QT_VERSION < 5
			val[c] = QStringRef(&data, vp - sp, cp - vp).toDouble(&res);
#endif // QT_VERSION < 5
			if (!res)
				return false;

			if (c == 1) {
				trackpoint.setCoordinates(Coordinates(val[0], val[1]));
				if (!trackpoint.coordinates().isValid())
					return false;
			} else if (c == 2)
				trackpoint.setElevation(val[2]);

			while (cp->isSpace())
				cp++;
			vp = cp;
			c++;
		}
	}

	return true;
}

bool KMLParser::pointCoordinates(Waypoint &waypoint)
{
	QString data = _reader.readElementText();
	const QChar *sp, *ep, *cp, *vp;
	int c = 0;
	qreal val[3];
	bool res;


	sp = data.constData();
	ep = sp + data.size();

	for (cp = sp; cp < ep; cp++)
		if (!cp->isSpace())
			break;

	for (vp = cp; cp <= ep; cp++) {
		if (*cp == ',') {
			if (c > 1)
				return false;

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
			val[c] = QString(vp, cp - vp).toDouble(&res);
#else // QT_VERSION < 5
			val[c] = QStringRef(&data, vp - sp, cp - vp).toDouble(&res);
#endif // QT_VERSION < 5
			if (!res)
				return false;

			c++;
			vp = cp + 1;
		} else if (cp->isSpace() || cp->isNull()) {
			if (c < 1)
				return false;

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
			val[c] = QString(vp, cp - vp).toDouble(&res);
#else // QT_VERSION < 5
			val[c] = QStringRef(&data, vp - sp, cp - vp).toDouble(&res);
#endif // QT_VERSION < 5
			if (!res)
				return false;

			waypoint.setCoordinates(Coordinates(val[0], val[1]));
			if (!waypoint.coordinates().isValid())
				return false;
			if (c == 2)
				waypoint.setElevation(val[2]);

			while (cp->isSpace())
				cp++;
			c = 3;
		}
	}

	return true;
}

bool KMLParser::lineCoordinates(TrackData &track)
{
	QString data = _reader.readElementText();
	const QChar *sp, *ep, *cp, *vp;
	int c = 0;
	qreal val[3];
	bool res;


	sp = data.constData();
	ep = sp + data.size();

	for (cp = sp; cp < ep; cp++)
		if (!cp->isSpace())
			break;

	for (vp = cp; cp <= ep; cp++) {
		if (*cp == ',') {
			if (c > 1)
				return false;

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
			val[c] = QString(vp, cp - vp).toDouble(&res);
#else // QT_VERSION < 5
			val[c] = QStringRef(&data, vp - sp, cp - vp).toDouble(&res);
#endif // QT_VERSION < 5
			if (!res)
				return false;

			c++;
			vp = cp + 1;
		} else if (cp->isSpace() || cp->isNull()) {
			if (c < 1 || c > 2)
				return false;

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
			val[c] = QString(vp, cp - vp).toDouble(&res);
#else // QT_VERSION < 5
			val[c] = QStringRef(&data, vp - sp, cp - vp).toDouble(&res);
#endif // QT_VERSION < 5
			if (!res)
				return false;

			track.append(Trackpoint(Coordinates(val[0], val[1])));
			if (!track.last().coordinates().isValid())
				return false;
			if (c == 2)
				track.last().setElevation(val[2]);

			while (cp->isSpace())
				cp++;
			c = 0;
			vp = cp;
		}
	}

 	return true;
}

QDateTime KMLParser::timeStamp()
{
	QDateTime ts;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == "when")
			ts = time();
		else
			_reader.skipCurrentElement();
	}

	return ts;
}

void KMLParser::lineString(TrackData &track)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "coordinates") {
			if (!lineCoordinates(track))
				_reader.raiseError("Invalid coordinates");
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::point(Waypoint &waypoint)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "coordinates") {
			if (!pointCoordinates(waypoint))
				_reader.raiseError("Invalid coordinates");
		} else
			_reader.skipCurrentElement();
	}

	if (waypoint.coordinates().isNull())
		_reader.raiseError("Missing Point coordinates");
}

void KMLParser::heartRate(TrackData &track, int start)
{
	int i = start;
	const char error[] = "Heartrate data count mismatch";

	while (_reader.readNextStartElement()) {
		if (_reader.name() == "value") {
			if (i < track.size())
				track[i++].setHeartRate(number());
			else {
				_reader.raiseError(error);
				return;
			}
		} else
			_reader.skipCurrentElement();
	}

	if (i != track.size())
		_reader.raiseError(error);
}

void KMLParser::cadence(TrackData &track, int start)
{
	int i = start;
	const char error[] = "Cadence data count mismatch";

	while (_reader.readNextStartElement()) {
		if (_reader.name() == "value") {
			if (i < track.size())
				track[i++].setCadence(number());
			else {
				_reader.raiseError(error);
				return;
			}
		} else
			_reader.skipCurrentElement();
	}

	if (i != track.size())
		_reader.raiseError(error);
}

void KMLParser::speed(TrackData &track, int start)
{
	int i = start;
	const char error[] = "Speed data count mismatch";

	while (_reader.readNextStartElement()) {
		if (_reader.name() == "value") {
			if (i < track.size())
				track[i++].setSpeed(number());
			else {
				_reader.raiseError(error);
				return;
			}
		} else
			_reader.skipCurrentElement();
	}

	if (i != track.size())
		_reader.raiseError(error);
}

void KMLParser::temperature(TrackData &track, int start)
{
	int i = start;
	const char error[] = "Temperature data count mismatch";

	while (_reader.readNextStartElement()) {
		if (_reader.name() == "value") {
			if (i < track.size())
				track[i++].setTemperature(number());
			else {
				_reader.raiseError(error);
				return;
			}
		} else
			_reader.skipCurrentElement();
	}

	if (i != track.size())
		_reader.raiseError(error);
}

void KMLParser::schemaData(TrackData &track, int start)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "SimpleArrayData") {
			QXmlStreamAttributes attr = _reader.attributes();
			QStringRef name = attr.value("name");

			if (name == "Heartrate")
				heartRate(track, start);
			else if (name == "Cadence")
				cadence(track, start);
			else if (name == "Speed")
				speed(track, start);
			else if (name == "Temperature")
				temperature(track, start);
			else
				_reader.skipCurrentElement();
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::extendedData(TrackData &track, int start)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "SchemaData")
			schemaData(track, start);
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::track(TrackData &track)
{
	const char error[] = "gx:coord/when element count mismatch";
	int first = track.size();
	int i = first;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == "when") {
			track.append(Trackpoint());
			track.last().setTimestamp(time());
		} else if (_reader.name() == "coord") {
			if (i == track.size()) {
				_reader.raiseError(error);
				return;
			} else if (!coord(track[i])) {
				_reader.raiseError("Invalid coordinates");
				return;
			}
			i++;
		} else if (_reader.name() == "ExtendedData")
			extendedData(track, first);
		else
			_reader.skipCurrentElement();
	}

	if (i != track.size())
		_reader.raiseError(error);
}

void KMLParser::multiTrack(TrackData &t)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Track")
			track(t);
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::multiGeometry(QList<TrackData> &tracks,
  QList<Waypoint> &waypoints, const QString &name, const QString &desc,
  const QDateTime timestamp)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Point") {
			waypoints.append(Waypoint());
			Waypoint &w = waypoints.last();
			w.setName(name);
			w.setDescription(desc);
			w.setTimestamp(timestamp);
			point(w);
		} else if (_reader.name() == "LineString") {
			tracks.append(TrackData());
			TrackData &t = tracks.last();
			t.setName(name);
			t.setDescription(desc);
			lineString(t);
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::placemark(QList<TrackData> &tracks, QList<Waypoint> &waypoints)
{
	QString name, desc;
	QDateTime timestamp;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == "name")
			name = _reader.readElementText();
		else if (_reader.name() == "description")
			desc = _reader.readElementText();
		else if (_reader.name() == "TimeStamp")
			timestamp = timeStamp();
		else if (_reader.name() == "MultiGeometry")
			multiGeometry(tracks, waypoints, name, desc, timestamp);
		else if (_reader.name() == "Point") {
			waypoints.append(Waypoint());
			Waypoint &w = waypoints.last();
			w.setName(name);
			w.setDescription(desc);
			w.setTimestamp(timestamp);
			point(w);
		} else if (_reader.name() == "LineString"
		  || _reader.name() == "LinearRing") {
			tracks.append(TrackData());
			TrackData &t = tracks.last();
			t.setName(name);
			t.setDescription(desc);
			lineString(t);
		} else if (_reader.name() == "Track") {
			tracks.append(TrackData());
			TrackData &t = tracks.last();
			t.setName(name);
			t.setDescription(desc);
			track(t);
		} else if (_reader.name() == "MultiTrack") {
			tracks.append(TrackData());
			TrackData &t = tracks.last();
			t.setName(name);
			t.setDescription(desc);
			multiTrack(t);
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::folder(QList<TrackData> &tracks, QList<Waypoint> &waypoints)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Placemark")
			placemark(tracks, waypoints);
		else if (_reader.name() == "Folder")
			folder(tracks, waypoints);
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::document(QList<TrackData> &tracks, QList<Waypoint> &waypoints)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Placemark")
			placemark(tracks, waypoints);
		else if (_reader.name() == "Folder")
			folder(tracks, waypoints);
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::kml(QList<TrackData> &tracks, QList<Waypoint> &waypoints)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Document")
			document(tracks, waypoints);
		else if (_reader.name() == "Placemark")
			placemark(tracks, waypoints);
		else if (_reader.name() == "Folder")
			folder(tracks, waypoints);
		else
			_reader.skipCurrentElement();
	}
}

bool KMLParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Waypoint> &waypoints)
{
	Q_UNUSED(routes);

	_reader.clear();
	_reader.setDevice(file);

	if (_reader.readNextStartElement()) {
		if (_reader.name() == "kml")
			kml(tracks, waypoints);
		else
			_reader.raiseError("Not a KML file");
	}

	return !_reader.error();
}
