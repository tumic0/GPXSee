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

	for (cp = sp, vp = sp; cp <= ep; cp++) {
		if (*cp == ',' || cp->isNull()) {
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

	for (cp = sp, vp = sp; cp <= ep; cp++) {
		if (*cp == ',' || cp->isSpace() || cp->isNull()) {
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
				_route->append(w);
			}

			c++;
			vp = cp + 1;
		}
		if (cp->isSpace()) {
			if (c < 3)
				return false;
			while (cp->isSpace())
				cp++;
			c = 0;
		}
	}

	return true;
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

void KMLParser::point()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "coordinates") {
			if (!pointCoordinates())
				_reader.raiseError("Invalid coordinates.");
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::multiGeometry()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Point")
			point();
		else if (_reader.name() == "LineString")
			lineString();
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::placemark()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Point")
			point();
		else if (_reader.name() == "LineString")
			lineString();
		else if (_reader.name() == "MultiGeometry")
			multiGeometry();
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::document()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Placemark")
			placemark();
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::kml()
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == "Document")
			document();
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
