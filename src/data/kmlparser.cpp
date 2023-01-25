/*
	WARNING: This code uses internal Qt API - the QZipReader class for reading
	ZIP files - and things may break if Qt changes the API. For Qt5 this is not
	a problem as we can "see the future" now and there are no changes in all
	the supported Qt5 versions up to the last one (5.15). In Qt6 the class
	might change or even disappear in the future, but this is very unlikely
	as there were no changes for several years and The Qt Company's policy
	is: "do not invest any resources into any desktop related stuff unless
	absolutely necessary". There is an issue (QTBUG-3897) since the year 2009 to
	include the ZIP reader into the public API, which aptly illustrates the
	effort The Qt Company is willing to make about anything desktop related...
*/

#include <QFileInfo>
#include <QTemporaryDir>
#include <QCryptographicHash>
#include <QtEndian>
#include <QUrl>
#include <QRegularExpression>
#include <private/qzipreader_p.h>
#include "common/util.h"
#include "kmlparser.h"

static bool isZIP(QFile *file)
{
	quint32 magic;

	return (file->read((char *)&magic, sizeof(magic)) == (qint64)sizeof(magic)
	  && qFromLittleEndian(magic) == 0x04034b50);
}

qreal KMLParser::number()
{
	bool res;
	QString str(_reader.readElementText());

	if (str.isEmpty())
		return NAN;

	qreal ret = str.toDouble(&res);
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

	if (data.isEmpty())
		return true;

	sp = data.constData();
	ep = sp + data.size();

	for (cp = sp; cp < ep; cp++)
		if (!cp->isSpace())
			break;

	for (vp = cp; cp <= ep; cp++) {
		if (cp->isSpace() || cp->isNull()) {
			if (c > 2)
				return false;

			val[c] = QString(vp, cp - vp).toDouble(&res);
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

			val[c] = QString(vp, cp - vp).toDouble(&res);
			if (!res)
				return false;

			c++;
			vp = cp + 1;
		} else if (cp->isSpace() || cp->isNull()) {
			if (c < 1)
				return false;

			val[c] = QString(vp, cp - vp).toDouble(&res);
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

			val[c] = QString(vp, cp - vp).toDouble(&res);
			if (!res)
				return false;

			c++;
			vp = cp + 1;
		} else if (cp->isSpace() || cp->isNull()) {
			if (c < 1 || c > 2)
				return false;

			val[c] = QString(vp, cp - vp).toDouble(&res);
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

			val[c] = QString(vp, cp - vp).toDouble(&res);
			if (!res)
				return false;

			c++;
			vp = cp + 1;
		} else if (cp->isSpace() || cp->isNull()) {
			if (c < 1 || c > 2)
				return false;

			val[c] = QString(vp, cp - vp).toDouble(&res);
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
	Polygon polygon;

	while (_reader.readNextStartElement()) {
		QVector<Coordinates> path;

		if (_reader.name() == QLatin1String("outerBoundaryIs")) {
			if (!polygon.isEmpty()) {
				_reader.raiseError("Multiple polygon outerBoundaryIss");
				return;
			}
			boundary(path);
			polygon.append(path);
		} else if (_reader.name() == QLatin1String("innerBoundaryIs")) {
			if (polygon.isEmpty()) {
				_reader.raiseError("Missing polygon outerBoundaryIs");
				return;
			}
			boundary(path);
			polygon.append(path);
		} else
			_reader.skipCurrentElement();
	}

	area.append(polygon);
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

void KMLParser::heartRate(SegmentData &segment)
{
	int i = 0;
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

	if (!_reader.error() && i != segment.size())
		_reader.raiseError(error);
}

void KMLParser::cadence(SegmentData &segment)
{
	int i = 0;
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

	if (!_reader.error() && i != segment.size())
		_reader.raiseError(error);
}

void KMLParser::speed(SegmentData &segment)
{
	int i = 0;
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

	if (!_reader.error() && i != segment.size())
		_reader.raiseError(error);
}

void KMLParser::temperature(SegmentData &segment)
{
	int i = 0;
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

	if (!_reader.error() && i != segment.size())
		_reader.raiseError(error);
}

void KMLParser::power(SegmentData &segment)
{
	int i = 0;
	const char error[] = "Power data count mismatch";

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("value")) {
			if (i < segment.size())
				segment[i++].setPower(number());
			else {
				_reader.raiseError(error);
				return;
			}
		} else
			_reader.skipCurrentElement();
	}

	if (!_reader.error() && i != segment.size())
		_reader.raiseError(error);
}

void KMLParser::schemaData(SegmentData &segment)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("SimpleArrayData")) {
			QXmlStreamAttributes attr = _reader.attributes();
			// There are files using capitalized names in the wild!
			QString name(attr.value("name").toString().toLower());

			if (name == QLatin1String("heartrate"))
				heartRate(segment);
			else if (name == QLatin1String("cadence"))
				cadence(segment);
			else if (name == QLatin1String("speed"))
				speed(segment);
			else if (name == QLatin1String("temperature"))
				temperature(segment);
			else if (name == QLatin1String("power"))
				power(segment);
			else
				_reader.skipCurrentElement();
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::extendedData(SegmentData &segment)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("SchemaData"))
			schemaData(segment);
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::track(SegmentData &segment)
{
	const char error[] = "gx:coord/when element count mismatch";
	int i = 0;
	bool empty = false;

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
			if (segment.at(i).coordinates().isNull())
				empty = true;
			i++;
		} else if (_reader.name() == QLatin1String("ExtendedData"))
			extendedData(segment);
		else
			_reader.skipCurrentElement();
	}

	if (i != segment.size()) {
		_reader.raiseError(error);
		return;
	}

	/* empty (invalid) coordinates are allowed per KML specification, but
	   invalid in our data representation so get rid of the segment entries */
	if (empty) {
		SegmentData filtered;
		for (int i = 0; i < segment.size(); i++)
			if (!segment.at(i).coordinates().isNull())
				filtered.append(segment.at(i));
		segment = filtered;
	}
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

void KMLParser::photoOverlay(const Ctx &ctx, QVector<Waypoint> &waypoints,
  PointStyleMap &pointStyles, QMap<QString, QString> &map)
{
	QString img, id;
	Waypoint w;
	QMap<QString, PolygonStyle> unused;
	QMap<QString, LineStyle> unused2;
	static QRegularExpression re("\\$\\[[^\\]]+\\]");

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("name"))
			w.setName(_reader.readElementText());
		else if (_reader.name() == QLatin1String("description"))
			w.setDescription(_reader.readElementText());
		else if (_reader.name() == QLatin1String("phoneNumber"))
			w.setPhone(_reader.readElementText());
		else if (_reader.name() == QLatin1String("address"))
			w.setAddress(_reader.readElementText());
		else if (_reader.name() == QLatin1String("TimeStamp"))
			w.setTimestamp(timeStamp());
		else if (_reader.name() == QLatin1String("Style")) {
			style(ctx.dir, pointStyles, unused, unused2);
			id = QString();
		} else if (_reader.name() == QLatin1String("StyleMap"))
			styleMap(map);
		else if (_reader.name() == QLatin1String("Icon"))
			img = icon();
		else if (_reader.name() == QLatin1String("Point"))
			point(w);
		else if (_reader.name() == QLatin1String("styleUrl"))
			id = styleUrl();
		else
			_reader.skipCurrentElement();
	}

	if (!w.coordinates().isNull()) {
		PointStyleMap::iterator pit = pointStyles.find(id);
		if (pit == pointStyles.end())
			pit = pointStyles.find(map.value(id));
		w.setStyle(pit == pointStyles.end()
		  ? PointStyle(QColor(0x55, 0x55, 0x55)) : *pit);

		img.replace(re, "0");
		if (!QUrl(img).scheme().isEmpty())
			w.addLink(Link(img, "Photo Overlay"));
		else if (ctx.zip && Util::tempDir().isValid()) {
			QFileInfo fi(img);
			QByteArray id(ctx.path.toUtf8() + img.toUtf8());
			QString path(Util::tempDir().path() + "/" + QString("%0.%1")
			  .arg(QCryptographicHash::hash(id, QCryptographicHash::Sha1)
			  .toHex(), QString(fi.suffix())));
			QFile::rename(ctx.dir.absoluteFilePath(img), path);
			w.addImage(path);
		} else if (!ctx.zip)
			w.addImage(ctx.dir.absoluteFilePath(img));

		waypoints.append(w);
	}
}

void KMLParser::multiGeometry(QList<TrackData> &tracks, QList<Area> &areas,
  QVector<Waypoint> &waypoints)
{
	TrackData *tp = 0;
	Area *ap = 0;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Point")) {
			waypoints.append(Waypoint());
			Waypoint &w = waypoints.last();
			point(w);
		} else if (_reader.name() == QLatin1String("LineString")) {
			if (!tp) {
				tracks.append(TrackData());
				tp = &tracks.last();
			}
			tp->append(SegmentData());
			lineString(tp->last());
		} else if (_reader.name() == QLatin1String("Polygon")) {
			if (!ap) {
				areas.append(Area());
				ap = &areas.last();
			}
			polygon(*ap);
		} else
			_reader.skipCurrentElement();
	}
}

void KMLParser::placemark(const Ctx &ctx, QList<TrackData> &tracks,
  QList<Area> &areas, QVector<Waypoint> &waypoints, PointStyleMap &pointStyles,
  PolygonStyleMap &polyStyles, LineStyleMap &lineStyles,
  QMap<QString, QString> &map)
{
	QString name, desc, phone, address, id;
	QDateTime timestamp;
	int trkIdx = tracks.size();
	int wptIdx = waypoints.size();
	int areaIdx = areas.size();

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("name"))
			name = _reader.readElementText();
		else if (_reader.name() == QLatin1String("description"))
			desc = _reader.readElementText();
		else if (_reader.name() == QLatin1String("phoneNumber"))
			phone = _reader.readElementText();
		else if (_reader.name() == QLatin1String("address"))
			address = _reader.readElementText();
		else if (_reader.name() == QLatin1String("TimeStamp"))
			timestamp = timeStamp();
		else if (_reader.name() == QLatin1String("Style")) {
			style(ctx.dir, pointStyles, polyStyles, lineStyles);
			id = QString();
		} else if (_reader.name() == QLatin1String("StyleMap"))
			styleMap(map);
		else if (_reader.name() == QLatin1String("MultiGeometry"))
			multiGeometry(tracks, areas, waypoints);
		else if (_reader.name() == QLatin1String("Point")) {
			waypoints.append(Waypoint());
			point(waypoints.last());
		} else if (_reader.name() == QLatin1String("LineString")
		  || _reader.name() == QLatin1String("LinearRing")) {
			tracks.append(TrackData());
			tracks.last().append(SegmentData());
			lineString(tracks.last().last());
		} else if (_reader.name() == QLatin1String("Track")) {
			tracks.append(TrackData());
			tracks.last().append(SegmentData());
			track(tracks.last().last());
		} else if (_reader.name() == QLatin1String("MultiTrack")) {
			tracks.append(TrackData());
			multiTrack(tracks.last());
		} else if (_reader.name() == QLatin1String("Polygon")) {
			areas.append(Area());
			polygon(areas.last());
		} else if (_reader.name() == QLatin1String("styleUrl"))
			id = styleUrl();
		else
			_reader.skipCurrentElement();
	}

	PointStyleMap::iterator pit = pointStyles.find(id);
	if (pit == pointStyles.end())
		pit = pointStyles.find(map.value(id));
	LineStyleMap::iterator lit = lineStyles.find(id);
	if (lit == lineStyles.end())
		lit = lineStyles.find(map.value(id));
	PolygonStyleMap::iterator ait = polyStyles.find(id);
	if (ait == polyStyles.end())
		ait = polyStyles.find(map.value(id));

	for (int i = wptIdx; i < waypoints.size(); i++) {
		Waypoint &w = waypoints[i];
		w.setName(name);
		w.setDescription(desc);
		w.setTimestamp(timestamp);
		w.setAddress(address);
		w.setPhone(phone);
		w.setStyle(pit == pointStyles.end()
		  ? PointStyle(QColor(0x55, 0x55, 0x55)) : *pit);
	}
	for (int i = trkIdx; i < tracks.size(); i++) {
		TrackData &t = tracks[i];
		t.setName(name);
		t.setDescription(desc);
		t.setStyle(lit == lineStyles.end()
		  ? LineStyle(QColor(0x55, 0x55, 0x55), 2, Qt::SolidLine) : *lit);
	}
	for (int i = areaIdx; i < areas.size(); i++) {
		Area &a = areas[i];
		a.setName(name);
		a.setDescription(desc);
		a.setStyle(ait == polyStyles.end()
		  ? PolygonStyle(QColor(0x55, 0x55, 0x55, 0x99),
		  QColor(0x55, 0x55, 0x55, 0x99), 2) : *ait);
	}
}

QString KMLParser::icon()
{
	QString path;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("href"))
			path = _reader.readElementText();
		else
			_reader.skipCurrentElement();
	}

	return path;
}

QColor KMLParser::color()
{
	QString str(_reader.readElementText());
	if (str.size() != 8)
		return QColor();

	bool aok, bok, gok, rok;
	int a = str.mid(0, 2).toInt(&aok, 16);
	int b = str.mid(2, 2).toInt(&bok, 16);
	int g = str.mid(4, 2).toInt(&gok, 16);
	int r = str.mid(6, 2).toInt(&rok, 16);

	return (aok && bok && gok && rok) ? QColor(r, g, b, a) : QColor();
}

QString KMLParser::styleUrl()
{
	QString id(_reader.readElementText());
	return (id.at(0) == '#') ? id.right(id.size() - 1) : QString();
}

void KMLParser::iconStyle(const QDir &dir, const QString &id,
  PointStyleMap &styles)
{
	QPixmap img;
	QColor c(0x55, 0x55, 0x55);

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Icon"))
			img = QPixmap(dir.absoluteFilePath(icon()));
		else if (_reader.name() == QLatin1String("color"))
			c = color();
		else
			_reader.skipCurrentElement();
	}

	styles.insert(id, PointStyle(img, c));
}

void KMLParser::polyStyle(const QString &id, PolygonStyleMap &styles)
{
	QColor c(0x55, 0x55, 0x55, 0x99);
	uint fill = 1, outline = 1;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("color"))
			c = color();
		else if (_reader.name() == QLatin1String("fill"))
			fill = _reader.readElementText().toUInt();
		else if (_reader.name() == QLatin1String("outline"))
			outline = _reader.readElementText().toUInt();
		else
			_reader.skipCurrentElement();
	}

	styles.insert(id, PolygonStyle(fill ? c : QColor(),
	  outline ? c : QColor(), 2));
}

void KMLParser::lineStyle(const QString &id, LineStyleMap &styles)
{
	QColor c(0x55, 0x55, 0x55);
	double width = 2;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("color"))
			c = color();
		else if (_reader.name() == QLatin1String("width"))
			width = _reader.readElementText().toDouble();
		else
			_reader.skipCurrentElement();
	}

	styles.insert(id, LineStyle(c, width, Qt::SolidLine));
}

void KMLParser::styleMapPair(const QString &id, QMap<QString, QString> &map)
{
	QString key, url;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("key"))
			key = _reader.readElementText();
		else if (_reader.name() == QLatin1String("styleUrl"))
			url = styleUrl();
		else
			_reader.skipCurrentElement();
	}

	if (key == "normal")
		map.insert(id, url);
}

void KMLParser::styleMap(QMap<QString, QString> &map)
{
	QString id = _reader.attributes().value("id").toString();

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Pair"))
			styleMapPair(id, map);
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::style(const QDir &dir, PointStyleMap &pointStyles,
  PolygonStyleMap &polyStyles, LineStyleMap &lineStyles)
{
	QString id = _reader.attributes().value("id").toString();

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("IconStyle"))
			iconStyle(dir, id, pointStyles);
		else if (_reader.name() == QLatin1String("PolyStyle"))
			polyStyle(id, polyStyles);
		else if (_reader.name() == QLatin1String("LineStyle"))
			lineStyle(id, lineStyles);
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::folder(const Ctx &ctx, QList<TrackData> &tracks,
  QList<Area> &areas, QVector<Waypoint> &waypoints,
  PointStyleMap &pointStyles, PolygonStyleMap &polyStyles,
  LineStyleMap &lineStyles, QMap<QString, QString> &map)
{
	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Document"))
			document(ctx, tracks, areas, waypoints);
		else if (_reader.name() == QLatin1String("Folder"))
			folder(ctx, tracks, areas, waypoints, pointStyles, polyStyles,
			  lineStyles, map);
		else if (_reader.name() == QLatin1String("Placemark"))
			placemark(ctx, tracks, areas, waypoints, pointStyles, polyStyles,
			  lineStyles, map);
		else if (_reader.name() == QLatin1String("PhotoOverlay"))
			photoOverlay(ctx, waypoints, pointStyles, map);
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::document(const Ctx &ctx, QList<TrackData> &tracks,
  QList<Area> &areas, QVector<Waypoint> &waypoints)
{
	PointStyleMap pointStyles;
	PolygonStyleMap polyStyles;
	LineStyleMap lineStyles;
	QMap<QString, QString> map;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Document"))
			document(ctx, tracks, areas, waypoints);
		else if (_reader.name() == QLatin1String("Folder"))
			folder(ctx, tracks, areas, waypoints, pointStyles, polyStyles,
			  lineStyles, map);
		else if (_reader.name() == QLatin1String("Placemark"))
			placemark(ctx, tracks, areas, waypoints, pointStyles, polyStyles,
			  lineStyles, map);
		else if (_reader.name() == QLatin1String("PhotoOverlay"))
			photoOverlay(ctx, waypoints, pointStyles, map);
		else if (_reader.name() == QLatin1String("Style"))
			style(ctx.dir, pointStyles, polyStyles, lineStyles);
		else if (_reader.name() == QLatin1String("StyleMap"))
			styleMap(map);
		else
			_reader.skipCurrentElement();
	}
}

void KMLParser::kml(const Ctx &ctx, QList<TrackData> &tracks,
  QList<Area> &areas, QVector<Waypoint> &waypoints)
{
	PointStyleMap pointStyles;
	PolygonStyleMap polyStyles;
	LineStyleMap lineStyles;
	QMap<QString, QString> map;

	while (_reader.readNextStartElement()) {
		if (_reader.name() == QLatin1String("Document"))
			document(ctx, tracks, areas, waypoints);
		else if (_reader.name() == QLatin1String("Folder"))
			folder(ctx, tracks, areas, waypoints, pointStyles, polyStyles,
			  lineStyles, map);
		else if (_reader.name() == QLatin1String("Placemark"))
			placemark(ctx, tracks, areas, waypoints, pointStyles, polyStyles,
			  lineStyles, map);
		else if (_reader.name() == QLatin1String("PhotoOverlay"))
			photoOverlay(ctx, waypoints, pointStyles, map);
		else
			_reader.skipCurrentElement();
	}
}

bool KMLParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &areas, QVector<Waypoint> &waypoints)
{
	Q_UNUSED(routes);
	QFileInfo fi(*file);

	_reader.clear();

	if (isZIP(file)) {
		QZipReader zip(fi.absoluteFilePath(), QIODevice::ReadOnly);
		QTemporaryDir tempDir;
		if (!tempDir.isValid() || !zip.extractAll(tempDir.path()))
			_reader.raiseError("Error extracting ZIP file");
		else {
			QDir zipDir(tempDir.path());
			QFileInfoList files(zipDir.entryInfoList(QStringList("*.kml"),
			  QDir::Files));

			if (files.isEmpty())
				_reader.raiseError("No KML file found in ZIP file");
			else {
				QFile kmlFile(files.first().absoluteFilePath());
				if (!kmlFile.open(QIODevice::ReadOnly))
					_reader.raiseError("Error opening KML file");
				else {
					_reader.setDevice(&kmlFile);

					if (_reader.readNextStartElement()) {
						if (_reader.name() == QLatin1String("kml"))
							kml(Ctx(fi.absoluteFilePath(), zipDir, true),
							  tracks, areas, waypoints);
						else
							_reader.raiseError("Not a KML file");
					}
				}
			}
		}
	} else {
		file->reset();
		_reader.setDevice(file);

		if (_reader.readNextStartElement()) {
			if (_reader.name() == QLatin1String("kml"))
				kml(Ctx(fi.absoluteFilePath(), fi.absoluteDir(), false), tracks,
				  areas, waypoints);
			else
				_reader.raiseError("Not a KML file");
		}
	}

	return !_reader.error();
}
