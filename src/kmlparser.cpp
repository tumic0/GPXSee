#include "kmlparser.h"


bool KMLParser::pointCoordinates()
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

			if (c == 2) {
				Waypoint w(Coordinates(val[0], val[1]));
				w.setElevation(val[2]);
				_waypoints.append(w);
			}

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

			Waypoint w(Coordinates(val[0], val[1]));
			if (c == 2)
				w.setElevation(val[2]);
			_waypoints.append(w);

			while (cp->isSpace())
				cp++;
			c = 3;
		}
	}

	return true;
}

bool KMLParser::lineCoordinates()
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

			Waypoint w(Coordinates(val[0], val[1]));
			if (c == 2)
				w.setElevation(val[2]);
			_route->append(w);

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
	QDateTime date;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == "when") {
			date = QDateTime::fromString(_reader.readElementText(),
			  Qt::ISODate);
		} else
			_reader.skipCurrentElement();
	}

	return date;
}

void KMLParser::lineString()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "coordinates") {
			_routes.append(QVector<Waypoint>());
			_route = &_routes.back();
			if (!lineCoordinates())
				_reader.raiseError("Invalid coordinates.");
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::point(const QString &name, const QString &desc,
  const QDateTime timestamp)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "coordinates") {
			if (!pointCoordinates())
				_reader.raiseError("Invalid coordinates.");
			else {
				Waypoint &w = _waypoints.last();
				if (!name.isNull())
					w.setName(name);
				if (!desc.isNull())
					w.setDescription(desc);
				if (!timestamp.isNull())
					w.setTimestamp(timestamp);
			}
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::multiGeometry(const QString &name, const QString &desc,
  const QDateTime timestamp)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Point")
			point(name, desc, timestamp);
		else if (_reader.name() == "LineString")
			lineString();
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::placemark()
{
	QString name, desc;
	QDateTime date;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == "name")
			name = _reader.readElementText();
		else if (_reader.name() == "description")
			desc = _reader.readElementText();
		else if (_reader.name() == "TimeStamp")
			date = timeStamp();
		else if (_reader.name() == "Point")
			point(name, desc, date);
		else if (_reader.name() == "LineString")
			lineString();
		else if (_reader.name() == "MultiGeometry")
			multiGeometry(name, desc, date);
		else
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
