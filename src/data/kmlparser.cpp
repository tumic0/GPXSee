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

bool KMLParser::lineCoordinates(SegmentData &segment)
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

			segment.append(Trackpoint(Coordinates(val[0], val[1])));
			if (!segment.last().coordinates().isValid())
				return false;
			if (c == 2)
				segment.last().setElevation(val[2]);

			while (cp->isSpace())
				cp++;
			c = 0;
			vp = cp;
		}
	}

 	return true;
}

bool KMLParser::polygonCoordinates(QVector<Coordinates> &points)
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

			points.append(Coordinates(val[0], val[1]));
			if (!points.last().isValid())
				return false;

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
		if (_reader.name() == QLatin1String("when"))
			ts = time();
		else
			_reader.skipCurrentElement();
	}

	return ts;
}

void KMLParser::lineString(SegmentData &segment)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("coordinates")) {
			if (!lineCoordinates(segment))
				_reader.raiseError("Invalid coordinates");
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::linearRing(QVector<Coordinates> &coordinates)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("coordinates")) {
			if (!polygonCoordinates(coordinates))
				_reader.raiseError("Invalid coordinates");
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::boundary(QVector<Coordinates> &coordinates)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("LinearRing"))
			linearRing(coordinates);
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::polygon(Area &area)
{
	area.append(Polygon());
	Polygon &polygon = area.last();

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("outerBoundaryIs")) {
			if (!polygon.isEmpty()) {
				_reader.raiseError("Multiple polygon outerBoundaryIss");
				return;
			}
			polygon.append(QVector<Coordinates>());
			boundary(polygon.last());
		} else if (_reader.name() == QLatin1String("innerBoundaryIs")) {
			if (polygon.isEmpty()) {
				_reader.raiseError("Missing polygon outerBoundaryIs");
				return;
			}
			polygon.append(QVector<Coordinates>());
			boundary(polygon.last());
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::point(Waypoint &waypoint)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("coordinates")) {
			if (!pointCoordinates(waypoint))
				_reader.raiseError("Invalid coordinates");
		} else
			_reader.skipCurrentElement();
	}

	if (waypoint.coordinates().isNull())
		_reader.raiseError("Missing Point coordinates");
}

void KMLParser::heartRate(SegmentData &segment, int start)
{
	int i = start;
	const char error[] = "Heartrate data count mismatch";

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("value")) {
			if (i < segment.size())
				segment[i++].setHeartRate(number());
			else {
				_reader.raiseError(error);
				return;
			}
		} else
			_reader.skipCurrentElement();
	}

	if (i != segment.size())
		_reader.raiseError(error);
}

void KMLParser::cadence(SegmentData &segment, int start)
{
	int i = start;
	const char error[] = "Cadence data count mismatch";

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("value")) {
			if (i < segment.size())
				segment[i++].setCadence(number());
			else {
				_reader.raiseError(error);
				return;
			}
		} else
			_reader.skipCurrentElement();
	}

	if (i != segment.size())
		_reader.raiseError(error);
}

void KMLParser::speed(SegmentData &segment, int start)
{
	int i = start;
	const char error[] = "Speed data count mismatch";

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("value")) {
			if (i < segment.size())
				segment[i++].setSpeed(number());
			else {
				_reader.raiseError(error);
				return;
			}
		} else
			_reader.skipCurrentElement();
	}

	if (i != segment.size())
		_reader.raiseError(error);
}

void KMLParser::temperature(SegmentData &segment, int start)
{
	int i = start;
	const char error[] = "Temperature data count mismatch";

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("value")) {
			if (i < segment.size())
				segment[i++].setTemperature(number());
			else {
				_reader.raiseError(error);
				return;
			}
		} else
			_reader.skipCurrentElement();
	}

	if (i != segment.size())
		_reader.raiseError(error);
}

void KMLParser::schemaData(SegmentData &segment, int start)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("SimpleArrayData")) {
			QXmlStreamAttributes attr = _reader.attributes();
			QStringRef name = attr.value("name");

			if (name == QLatin1String("Heartrate"))
				heartRate(segment, start);
			else if (name == QLatin1String("Cadence"))
				cadence(segment, start);
			else if (name == QLatin1String("Speed"))
				speed(segment, start);
			else if (name == QLatin1String("Temperature"))
				temperature(segment, start);
			else
				_reader.skipCurrentElement();
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::extendedData(SegmentData &segment, int start)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("SchemaData"))
			schemaData(segment, start);
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::track(SegmentData &segment)
{
	const char error[] = "gx:coord/when element count mismatch";
	int first = segment.size();
	int i = first;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("when")) {
			segment.append(Trackpoint());
			segment.last().setTimestamp(time());
		} else if (_reader.name() == QLatin1String("coord")) {
			if (i == segment.size()) {
				_reader.raiseError(error);
				return;
			} else if (!coord(segment[i])) {
				_reader.raiseError("Invalid coordinates");
				return;
			}
			i++;
		} else if (_reader.name() == QLatin1String("ExtendedData"))
			extendedData(segment, first);
		else
			_reader.skipCurrentElement();
	}

	if (i != segment.size())
		_reader.raiseError(error);
}

void KMLParser::multiTrack(TrackData &t)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Track")) {
			t.append(SegmentData());
			track(t.last());
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::multiGeometry(QList<TrackData> &tracks, QList<Area> &areas,
  QVector<Waypoint> &waypoints, const QString &name, const QString &desc,
  const QDateTime &timestamp)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Point")) {
			waypoints.append(Waypoint());
			Waypoint &w = waypoints.last();
			w.setName(name);
			w.setDescription(desc);
			w.setTimestamp(timestamp);
			point(w);
		} else if (_reader.name() == QLatin1String("LineString")) {
			tracks.append(TrackData());
			TrackData &t = tracks.last();
			t.append(SegmentData());
			t.setName(name);
			t.setDescription(desc);
			lineString(t.last());
		} else if (_reader.name() == QLatin1String("Polygon")) {
			areas.append(Area());
			Area &a = areas.last();
			a.setName(name);
			a.setDescription(desc);
			polygon(a);
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::placemark(QList<TrackData> &tracks, QList<Area> &areas,
  QVector<Waypoint> &waypoints)
{
	QString name, desc;
	QDateTime timestamp;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("name"))
			name = _reader.readElementText();
		else if (_reader.name() == QLatin1String("description"))
			desc = _reader.readElementText();
		else if (_reader.name() == QLatin1String("TimeStamp"))
			timestamp = timeStamp();
		else if (_reader.name() == QLatin1String("MultiGeometry"))
			multiGeometry(tracks, areas, waypoints, name, desc, timestamp);
		else if (_reader.name() == QLatin1String("Point")) {
			waypoints.append(Waypoint());
			Waypoint &w = waypoints.last();
			w.setName(name);
			w.setDescription(desc);
			w.setTimestamp(timestamp);
			point(w);
		} else if (_reader.name() == QLatin1String("LineString")
		  || _reader.name() == QLatin1String("LinearRing")) {
			tracks.append(TrackData());
			TrackData &t = tracks.last();
			t.append(SegmentData());
			t.setName(name);
			t.setDescription(desc);
			lineString(t.last());
		} else if (_reader.name() == QLatin1String("Track")) {
			tracks.append(TrackData());
			TrackData &t = tracks.last();
			t.append(SegmentData());
			t.setName(name);
			t.setDescription(desc);
			track(t.last());
		} else if (_reader.name() == QLatin1String("MultiTrack")) {
			tracks.append(TrackData());
			TrackData &t = tracks.last();
			t.setName(name);
			t.setDescription(desc);
			multiTrack(t);
		} else if (_reader.name() == QLatin1String("Polygon")) {
			areas.append(Area());
			Area &a = areas.last();
			a.setName(name);
			a.setDescription(desc);
			polygon(a);
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::folder(QList<TrackData> &tracks, QList<Area> &areas,
  QVector<Waypoint> &waypoints)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Placemark"))
			placemark(tracks, areas, waypoints);
		else if (_reader.name() == QLatin1String("Folder"))
			folder(tracks, areas, waypoints);
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::document(QList<TrackData> &tracks, QList<Area> &areas,
  QVector<Waypoint> &waypoints)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Placemark"))
			placemark(tracks, areas, waypoints);
		else if (_reader.name() == QLatin1String("Folder"))
			folder(tracks, areas, waypoints);
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::kml(QList<TrackData> &tracks, QList<Area> &areas,
  QVector<Waypoint> &waypoints)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Document"))
			document(tracks, areas, waypoints);
		else if (_reader.name() == QLatin1String("Placemark"))
			placemark(tracks, areas, waypoints);
		else if (_reader.name() == QLatin1String("Folder"))
			folder(tracks, areas, waypoints);
		else
			_reader.skipCurrentElement();
	}
}

bool KMLParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &areas, QVector<Waypoint> &waypoints)
{
	Q_UNUSED(routes);

	_reader.clear();
	_reader.setDevice(file);

	if (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("kml"))
			kml(tracks, areas, waypoints);
		else
			_reader.raiseError("Not a KML file");
	}

	return !_reader.error();
}
