#include <cstring>
#include "common/str2int.h"
#include "igcparser.h"


static bool readLat(const char *data, qreal &lat)
{
	int d = str2int(data, 2);
	int mi = str2int(data + 2, 2);
	int mf = str2int(data + 4, 3);
	if (d < 0 || mi < 0 || mf < 0)
		return false;

	if (!(data[7] == 'N' || data[7] == 'S'))
		return false;

	lat = d + (((qreal)mi + (qreal)mf/1000) / 60);
	if (lat > 90)
		return false;

	if (data[7] == 'S')
		lat = -lat;

	return true;
}

static bool readLon(const char *data, qreal &lon)
{
	int d = str2int(data, 3);
	int mi = str2int(data + 3, 2);
	int mf = str2int(data + 5, 3);
	if (d < 0 || mi < 0 || mf < 0)
		return false;

	if (!(data[8] == 'E' || data[8] == 'W'))
		return false;

	lon = d + (((qreal)mi + (qreal)mf/1000) / 60);
	if (lon > 180)
		return false;

	if (data[8] == 'W')
		lon = -lon;

	return true;
}

static bool readAltitude(const char *data, qreal &ele)
{
	int ga;

	if (!(data[0] == 'A' || data[0] == 'V'))
		return false;

	if (data[6] == '-') {
		if ((ga = str2int(data + 7, 4)) < 0)
			return false;
		ga = -ga;
	} else {
		if ((ga = str2int(data + 6, 5)) < 0)
			return false;
	}

	if (data[0] == 'A')
		ele = (qreal)ga;
	else
		ele = NAN;

	return true;
}

static bool readTimestamp(const char *data, QTime &time)
{
	int h = str2int(data, 2);
	int m = str2int(data + 2, 2);
	int s = str2int(data + 4, 2);

	if (h < 0 || m < 0 || s < 0)
		return false;

	time = QTime(h, m, s);
	if (!time.isValid())
		return false;

	return true;
}

static bool readARecord(const char *line, qint64 len)
{
	if (len < 7 || line[0] != 'A')
		return false;

	for (int i = 1; i < 7; i++)
		if (!::isprint(line[i]))
			return false;
	return true;
}

bool IGCParser::readHRecord(const char *line, int len)
{
	if (len < 11 || ::strncmp(line, "HFDTE", 5))
		return true;

	int offset = (len < 16 || ::strncmp(line + 5, "DATE:", 5)) ? 5 : 10;

	int d = str2int(line + offset, 2);
	int m = str2int(line + offset + 2, 2);
	int y = str2int(line + offset + 4, 2);

	if (y < 0 || m < 0 || d < 0) {
		_errorString = "Invalid date header format";
		return false;
	}

	_date = QDate(y + 2000 < QDate::currentDate().year() ? 2000 + y : 1900 + y,
	  m, d);
	if (!_date.isValid()) {
		_errorString = "Invalid date";
		return false;
	}

	return true;
}

bool IGCParser::readBRecord(TrackData &track, const char *line, int len)
{
	qreal lat, lon, ele;
	QTime time;


	if (len < 35)
		return false;

	if (!readTimestamp(line + 1, time)) {
		_errorString = "Invalid timestamp";
		return false;
	}

	if (!readLat(line + 7, lat)) {
		_errorString = "Invalid latitude";
		return false;
	}
	if (!readLon(line + 15, lon)) {
		_errorString = "Invalid longitude";
		return false;
	}

	if (!readAltitude(line + 24, ele)) {
		_errorString = "Invalid altitude";
		return false;
	}


	if (time < _time)
		_date = _date.addDays(1);
	_time = time;

	Trackpoint t(Coordinates(lon, lat));
	t.setTimestamp(QDateTime(_date, _time, Qt::UTC));
	t.setElevation(ele);
	track.append(t);

	return true;
}

bool IGCParser::readCRecord(RouteData &route, const char *line, int len)
{
	qreal lat, lon;


	if (len < 18)
		return false;

	if (!readLat(line + 1, lat)) {
		_errorString = "Invalid latitude";
		return false;
	}
	if (!readLon(line + 9, lon)) {
		_errorString = "Invalid longitude";
		return false;
	}

	if (!(lat == 0 && lon == 0)) {
		QByteArray ba(line + 18, len - 19);

		Waypoint w(Coordinates(lon, lat));
		w.setName(QString(ba.trimmed()));
		route.append(w);
	}

	return true;
}

bool IGCParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Waypoint> &waypoints)
{
	Q_UNUSED(waypoints);
	qint64 len;
	char line[76 + 2 + 1 + 1];
	bool route = false, track = false;


	_errorLine = 1;
	_errorString.clear();

	while (!file->atEnd()) {
		len = file->readLine(line, sizeof(line));

		if (len < 0) {
			_errorString = "I/O error";
			return false;
		} else if (len > (qint64)sizeof(line) - 1) {
			_errorString = "Line limit exceeded";
			return false;
		}

		if (_errorLine == 1) {
			if (!readARecord(line, len)) {
				_errorString = "Invalid/missing A record";
				return false;
			}
		} else {
			if (line[0] == 'H') {
				if (!readHRecord(line, len))
					return false;
			} else if (line[0] == 'C') {
				if (route) {
					if (!readCRecord(routes.last() ,line, len))
						return false;
				} else {
					route = true;
					routes.append(RouteData());
				}
			} else if (line[0] == 'B') {
				if (_date.isNull()) {
					_errorString = "Missing date header";
					return false;
				}
				if (!track) {
					tracks.append(TrackData());
					_time = QTime(0, 0);
					track = true;
				}
				if (!readBRecord(tracks.last(), line, len))
					return false;
			}
		}

		_errorLine++;
	}

	return true;
}
