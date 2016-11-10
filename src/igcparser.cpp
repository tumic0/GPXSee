#include <cctype>
#include <cstring>
#include "igcparser.h"


static int str2int(const char *str, size_t len)
{
	int res = 0;

	for (const char *sp = str; sp < str + len; sp++) {
		if (::isdigit(*sp))
			res = res * 10 + *sp - '0';
		else
			return -1;
	}

	return res;
}

static bool readLat(const char *line, qreal &lat)
{
	int d = str2int(line + 7, 2);
	int mi = str2int(line + 9, 2);
	int mf = str2int(line + 11, 3);
	if (d < 0 || mi < 0 || mf < 0)
		return false;

	if (!(line[14] == 'N' || line[14] == 'S'))
		return false;

	lat = d + (((qreal)mi + (qreal)mf/1000) / 60);
	if (lat > 90)
		return false;

	if (line[14] == 'S')
		lat = -lat;

	return true;
}

static bool readLon(const char *line, qreal &lon)
{
	int d = str2int(line + 15, 3);
	int mi = str2int(line + 18, 2);
	int mf = str2int(line + 20, 3);
	if (d < 0 || mi < 0 || mf < 0)
		return false;

	if (!(line[23] == 'E' || line[23] == 'W'))
		return false;

	lon = d + (((qreal)mi + (qreal)mf/1000) / 60);
	if (lon > 180)
		return false;

	if (line[23] == 'W')
		lon = -lon;

	return true;
}

static bool readAltitude(const char *line, qreal &ele)
{
	int p;

	if (!(line[24] == 'A' || line[24] == 'V'))
		return false;

	if (line[25] == '-')
		p = str2int(line + 26, 4);
	else
		p = str2int(line + 25, 5);

	int g = str2int(line + 30, 5);
	if (p < 0 || g < 0)
		return false;

	if (line[24] == 'A')
		ele = (qreal)g;
	else
		ele = NAN;

	return true;
}

static bool readARecord(const char *line, qint64 len)
{
	if (len < 9 || line[0] != 'A')
		return false;

	for (int i = 1; i < 7; i++)
		if (!::isprint(line[i]))
			return false;
	return true;
}

bool IGCParser::readHRecord(const char *line, qint64 len)
{
	if (len < 10 || ::strncmp(line, "HFDTE", 5))
		return true;

	int d = str2int(line + 5, 2);
	int m = str2int(line + 7, 2);
	int y = str2int(line + 9, 2);

	if (y < 0 || m < 0 || d < 0) {
		_errorString = "Invalid date header format";
		return false;
	}

	_date = QDate(2000 + y, m, d);
	if (!_date.isValid()) {
		_errorString = "Invalid date";
		return false;
	}

	return true;
}

bool IGCParser::readBRecord(const char *line, qint64 len)
{
	qreal lat, lon, ele;
	QDateTime timestamp;


	if (len < 35)
		return false;

	int h = str2int(line + 1, 2);
	int m = str2int(line + 3, 2);
	int s = str2int(line + 5, 2);

	if (h <0 || m < 0 || s < 0) {
		_errorString = "Invalid  timestamp";
		return false;
	}
	QTime time = QTime(h, m, s);
	if (time < _time)
		_date.addDays(1);
	_time = time;
	timestamp = QDateTime(_date, _time, Qt::UTC);

	if (!readLat(line, lat)) {
		_errorString = "Invalid latitude";
		return false;
	}
	if (!readLon(line, lon)) {
		_errorString = "Invalid longitude";
		return false;
	}

	if (!readAltitude(line, ele)) {
		_errorString = "Invalid altitude";
		return false;
	}

	Trackpoint t(Coordinates(lon, lat));
	t.setTimestamp(timestamp);
	t.setElevation(ele);
	_tracks.last().append(t);

	return true;
}

bool IGCParser::loadFile(QFile *file)
{
	qint64 len;
	char line[76 + 2 + 1 + 1];

	_errorLine = 1;
	_errorString.clear();

	_tracks.append(TrackData());
	_time = QTime(0, 0);


	while ((len = file->readLine(line, sizeof(line))) > 0) {
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
			} else if (line[0] == 'B') {
				if (_date.isNull()) {
					_errorString = "Missing date header";
					return false;
				}
				if (!readBRecord(line, len))
					return false;
			}
		}

		_errorLine++;
	}

	return true;
}
