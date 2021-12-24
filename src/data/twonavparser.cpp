#include "common/textcodec.h"
#include "map/gcs.h"
#include "twonavparser.h"

static double lon(const QString &str)
{
	QStringList l(str.split(QChar(0xBA)));
	if (l.size() < 2)
		return NAN;

	bool ok;
	double val = l.at(0).toDouble(&ok);
	if (!ok)
		return NAN;

	if (l.at(1) == "W")
		return -val;
	else if (l.at(1) == "E")
		return val;
	else
		return NAN;
}

static double lat(const QString &str)
{
	QStringList l(str.split(QChar(0xBA)));
	if (l.size() < 2)
		return NAN;

	bool ok;
	double val = l.at(0).toDouble(&ok);
	if (!ok)
		return NAN;

	if (l.at(1) == "S")
		return -val;
	else if (l.at(1) == "N")
		return val;
	else
		return NAN;
}

static QDateTime timestamp(const QString &dateStr, const QString &timeStr)
{
	QLocale l("C");

	QDate date(l.toDate(dateStr, "dd-MMM-yy"));
	if (date.isValid())
		date = date.addYears(100);
	else {
		date = l.toDate(dateStr, "dd-MMM-yyyy");
		if (!date.isValid())
			return QDateTime();
	}

	QTime time(l.toTime(timeStr, "H:m:s.z"));
	if (!time.isValid()) {
		time = l.toTime(timeStr, "H:m:s");
		if (!time.isValid())
			return QDateTime();
	}

	return QDateTime(date, time);
}

bool TwoNavParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons,
  QVector<Waypoint> &waypoints)
{
	Q_UNUSED(polygons);
	TextCodec codec;
	GCS gcs;
	bool ok, route = false, track = false, waypoint = false;

	_errorLine = 1;
	_errorString.clear();

	quint8 bom[3];
	if (file->peek((char*)bom, sizeof(bom)) == sizeof(bom)
	  && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) {
		file->seek(3);
		codec = TextCodec(65001);
	}

	while (!file->atEnd()) {
		QByteArray line(file->readLine().trimmed());
		if (!line.size())
			continue;

		switch (line.at(0)) {
			case 'B':
				{line.remove(0, 1);
				QByteArray encoding(line.trimmed());
				if (encoding == "UTF-8")
					codec = TextCodec(65001);
				else {
					_errorString = "Invalid/unknown encoding";
					return false;
				}}
				break;
			case 'G':
				{line.remove(0, 1);
				QString datum(line.trimmed());
				gcs = GCS::gcs(datum);
				if (gcs.isNull()) {
					_errorString = "Invalid/unknown datum";
					return false;
				}}
				break;
			case 'U':
				{line.remove(0, 1);
				QByteArray cs(line.trimmed());
				if (cs != "1") {
					_errorString = "Invalid/unknown coordinate system";
					return false;
				}}
				break;
			case 'T':
				{QStringList list(codec.toString(line).split(' ',
				  Qt::SkipEmptyParts));
				if (list.size() < 4) {
					_errorString = "Parse error";
					return false;
				}
				Coordinates c(lon(list.at(3)), lat(list.at(2)));
				if (!c.isValid()) {
					_errorString = "Invalid coordinates";
					return false;
				}

				Trackpoint t(gcs.toWGS84(c));

				if (list.size() > 5) {
					QDateTime ts(timestamp(list.at(4), list.at(5)));
					if (!ts.isValid()) {
						_errorString = "Invalid date/time";
						return false;
					}
					t.setTimestamp(ts);
				}
				if (list.size() > 7) {
					qreal elevation = list.at(7).toDouble(&ok);
					if (!ok) {
						_errorString = "Invalid altitude";
						return false;
					}
					t.setElevation(elevation);
				}

				if (!track) {
					tracks.append(TrackData());
					tracks.last().append(SegmentData());
					track = true;
				}

				tracks.last().last().append(t);}
				break;
			case 'W':
				{QStringList list(codec.toString(line).split(' ',
				  Qt::SkipEmptyParts));
				if (list.size() < 5) {
					_errorString = "Parse error";
					return false;
				}
				Coordinates c(lon(list.at(4)), lat(list.at(3)));
				if (!c.isValid()) {
					_errorString = "Invalid coordinates";
					return false;
				}

				Waypoint w(gcs.toWGS84(c));
				w.setName(list.at(1));

				if (list.size() > 6) {
					QDateTime ts(timestamp(list.at(5), list.at(6)));
					if (!ts.isValid()) {
						_errorString = "Invalid date/time";
						return false;
					}
					w.setTimestamp(ts);
				}
				if (list.size() > 7) {
					qreal elevation = list.at(7).toDouble(&ok);
					if (!ok) {
						_errorString = "Invalid altitude";
						return false;
					}
					w.setElevation(elevation);
				}
				if (list.size() > 8)
					w.setDescription(list.mid(8).join(' '));

				if (route)
					routes.last().append(w);
				else {
					waypoints.append(w);
					waypoint = true;
				}}
				break;
			case 'R':
				{QStringList list(codec.toString(line).split(',',
				  Qt::SkipEmptyParts));
				routes.append(RouteData());
				routes.last().setName(list.at(1));
				route = true;}
				break;
		}

		_errorLine++;
	}

	if (!(waypoint | route | track)) {
		_errorString = "No valid data found";
		return false;
	} else
		return true;
}
