#include "kmlparser.h"


qreal KMLParser::number()
{
	bool res;
	qreal ret = _reader.readElementText().toDouble(&res);
	if (!res)
		_reader.raiseError(QString("Invalid %1.").arg(
		  _reader.name().toString()));

	return ret;
}

QDateTime KMLParser::time()
{
	QDateTime d = QDateTime::fromString(_reader.readElementText(),
	  Qt::ISODate);
	if (!d.isValid())
		_reader.raiseError(QString("Invalid %1.").arg(
		  _reader.name().toString()));

	return d;
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
			if (c > 2)
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
			if (c > 2)
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
				_reader.raiseError("Invalid coordinates.");
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::point(Waypoint &waypoint)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "coordinates") {
			if (!pointCoordinates(waypoint))
				_reader.raiseError("Invalid coordinates.");
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::placemark()
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
		else if (_reader.name() == "Point") {
			_waypoints.append(Waypoint());
			Waypoint &waypoint = _waypoints.last();
			waypoint.setName(name);
			waypoint.setDescription(desc);
			waypoint.setTimestamp(timestamp);
			point(waypoint);
		} else if (_reader.name() == "LineString") {
			_tracks.append(TrackData());
			TrackData &track = _tracks.last();
			track.setName(name);
			track.setDescription(desc);
			lineString(track);
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::folder()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Placemark")
			placemark();
		else if (_reader.name() == "Folder")
			folder();
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::document()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Placemark")
			placemark();
		else if (_reader.name() == "Folder")
			folder();
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::kml()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Document")
			document();
		else if (_reader.name() == "Placemark")
			placemark();
		else
			_reader.skipCurrentElement();
	}
}

bool KMLParser::parse()
{
	if (_reader.readNextStartElement()) {
		if (_reader.name() == "kml")
			kml();
		else
			_reader.raiseError("Not a KML file.");
	}

	return !_reader.error();
}

bool KMLParser::loadFile(QIODevice *device)
{
	_reader.clear();
	_reader.setDevice(device);

	return parse();
}
