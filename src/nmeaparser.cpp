#include <cstring>
#include "misc.h"
#include "nmeaparser.h"


static bool readTime(const char *data, int len, QTime &time)
{
	int h, m, s, ms = 0;

	if (len < 6)
		return false;

	h = str2int(data, 2);
	m = str2int(data + 2, 2);
	s = str2int(data + 4, 2);
	if (h < 0 || m < 0 || s < 0)
		return false;

	if (len > 7 && data[6] == '.') {
		if ((ms = str2int(data + 7, len - 7)) < 0)
			return false;
	}

	time = QTime(h, m, s, ms);
	if (!time.isValid())
		return false;

	return true;
}

static bool readDate(const char *data, int len, QDate &date)
{
	int y, m, d;

	if (len != 6)
		return false;

	d = str2int(data, 2);
	m = str2int(data + 2, 2);
	y = str2int(data + 4, 2);
	if (d < 0 || m < 0 || y < 0)
		return false;

	date = QDate(2000 + y, m, d);
	if (!date.isValid())
		return false;

	return true;
}

static bool readLat(const char *data, int len, qreal &lat)
{
	int d, mi;
	qreal mf;
	bool ok;

	if (len < 7 || data[4] != '.')
		return false;

	d = str2int(data, 2);
	mi = str2int(data + 2, 2);
	mf = QString(QByteArray::fromRawData(data + 4, len - 4)).toFloat(&ok);
	if (d < 0 || mi < 0 || !ok)
		return false;

	lat = d + (((qreal)mi + mf) / 60.0);
	if (lat > 90)
		return false;

	return true;
}

static bool readLon(const char *data, int len, qreal &lon)
{
	int d, mi;
	qreal mf;
	bool ok;

	if (len < 8 || data[5] != '.')
		return false;

	d = str2int(data, 3);
	mi = str2int(data + 3, 2);
	mf = QString(QByteArray::fromRawData(data + 5, len - 5)).toFloat(&ok);
	if (d < 0 || mi < 0 || !ok)
		return false;

	lon = d + (((qreal)mi + mf) / 60.0);
	if (lon > 180)
		return false;

	return true;
}

static bool readFloat(const char *data, int len, qreal &f)
{
	bool ok;

	f = QString(QByteArray::fromRawData(data, len)).toFloat(&ok);

	return ok;
}

static bool validSentence(const char  *line, int len)
{
	const char *lp;

	if (len < 10 || line[0] != '$')
		return false;

	for (lp = line + len - 1; lp > line; lp--)
		if (!::isspace(*lp))
			break;
	if (*(lp-2) != '*' || !::isalnum(*(lp-1)) || !::isalnum(*(lp)))
		return false;

	return true;
}

bool NMEAParser::readRMC(const char *line, int len)
{
	int col = 1;
	const char *vp = line;
	qreal lat, lon;
	QTime time;
	bool valid = true;

	for (const char *lp = line; lp < line + len; lp++) {
		if (*lp == ',' || *lp == '*') {
			switch (col) {
				case 1:
					if (!readTime(vp, lp - vp, time)) {
						_errorString = "Invalid time";
						return false;
					}
					break;
				case 2:
					if (*vp != 'A')
						valid = false;
					break;
				case 3:
					if (!readLat(vp, lp - vp, lat)) {
						_errorString = "Invalid latitude";
						return false;
					}
					break;
				case 4:
					if (!(*vp == 'N' || *vp == 'S')) {
						_errorString = "Invalid latitude N|S";
						return false;
					}
					if (*lp == 'S')
						lat = -lat;
					break;
				case 5:
					if (!readLon(vp, lp - vp, lon)) {
						_errorString = "Invalid longitude";
						return false;
					}
					break;
				case 6:
					if (!(*vp == 'E' || *vp == 'W')) {
						_errorString = "Invalid latitude E|W";
						return false;
					}
					if (*vp == 'W')
						lon = -lon;
					break;
				case 9:
					if (!readDate(vp, lp - vp, _date)) {
						_errorString = "Invalid date";
						return false;
					}
					break;
			}

			col++;
			vp = lp + 1;
		}
	}

	if (col < 9) {
		_errorString = "Invalid RMC sentence";
		return false;
	}

	if (valid && !_GGA) {
		Trackpoint t(Coordinates(lon, lat));
		t.setTimestamp(QDateTime(_date, time, Qt::UTC));
		_tracks.last().append(t);
	}

	return true;
}

bool NMEAParser::readGGA(const char *line, int len)
{
	int col = 1;
	const char *vp = line;
	qreal lat, lon, ele, gh;
	QTime time;

	for (const char *lp = line; lp < line + len; lp++) {
		if (*lp == ',' || *lp == '*') {
			switch (col) {
				case 1:
					if (!readTime(vp, lp - vp, time)) {
						_errorString = "Invalid time";
						return false;
					}
					break;
				case 2:
					if (!readLat(vp, lp - vp, lat)) {
						_errorString = "Invalid latitude";
						return false;
					}
					break;
				case 3:
					if (!(*vp == 'N' || *vp == 'S')) {
						_errorString = "Invalid latitude N|S";
						return false;
					}
					if (*vp == 'S')
						lat = -lat;
					break;
				case 4:
					if (!readLon(vp, lp - vp, lon)) {
						_errorString = "Invalid longitude";
						return false;
					}
					break;
				case 5:
					if (!(*vp == 'E' || *vp == 'W')) {
						_errorString = "Invalid latitude E|W";
						return false;
					}
					if (*vp == 'W')
						lon = -lon;
					break;
				case 9:
					if ((lp - vp) && !readFloat(vp, lp - vp, ele)) {
						_errorString = "Invalid altitude";
						return false;
					}
					break;
				case 10:
					if ((lp - vp) && *vp != 'M') {
						_errorString = "Invalid altitude units";
						return false;
					}
					break;
				case 11:
					if ((lp - vp) && !readFloat(vp, lp - vp, gh)) {
						_errorString = "Invalid geoid height";
						return false;
					}
					break;
				case 12:
					if ((lp - vp) && *vp != 'M') {
						_errorString = "Invalid geoid height units";
						return false;
					}
					break;
			}

			col++;
			vp = lp + 1;
		}
	}

	if (col < 12) {
		_errorString = "Invalid GGA sentence";
		return false;
	}

	_GGA = true;

	Trackpoint t(Coordinates(lon, lat));
	t.setTimestamp(QDateTime(_date, time, Qt::UTC));
	t.setElevation(ele - gh);
	_tracks.last().append(t);

	return true;
}

bool NMEAParser::readWPL(const char *line, int len)
{
	int col = 1;
	const char *vp = line;
	qreal lat, lon;
	QString name;

	for (const char *lp = line; lp < line + len; lp++) {
		if (*lp == ',' || *lp == '*') {
			switch (col) {
				case 1:
					if (!readLat(vp, lp - vp, lat)) {
						_errorString = "Invalid latitude";
						return false;
					}
					break;
				case 2:
					if (!(*vp == 'N' || *vp == 'S')) {
						_errorString = "Invalid latitude N|S";
						return false;
					}
					if (*vp == 'S')
						lat = -lat;
					break;
				case 3:
					if (!readLon(vp, lp - vp, lon)) {
						_errorString = "Invalid longitude";
						return false;
					}
					break;
				case 4:
					if (!(*vp == 'E' || *vp == 'W')) {
						_errorString = "Invalid latitude E|W";
						return false;
					}
					if (*vp == 'W')
						lon = -lon;
					break;
				case 5:
					name = QString(QByteArray(vp, lp - vp));
					break;
			}

			col++;
			vp = lp + 1;
		}
	}

	if (col < 4) {
		_errorString = "Invalid WPL sentence";
		return false;
	}

	Waypoint w(Coordinates(lon, lat));
	w.setName(name);
	_waypoints.append(w);

	return true;
}

bool NMEAParser::readZDA(const char *line, int len)
{
	int col = 1;
	const char *vp = line;
	int d, m, y;

	for (const char *lp = line; lp < line + len; lp++) {
		if (*lp == ',' || *lp == '*') {
			switch (col) {
				case 2:
					if ((d = str2int(vp, lp - vp)) < 0) {
						_errorString = "Invalid day";
						return false;
					}
					break;
				case 3:
					if ((m = str2int(vp, lp - vp)) < 0) {
						_errorString = "Invalid month";
						return false;
					}
					break;
				case 4:
					if ((y = str2int(vp, lp - vp)) < 0) {
						_errorString = "Invalid year";
						return false;
					}
					break;
			}

			col++;
			vp = lp + 1;
		}
	}

	if (col < 4) {
		_errorString = "Invalid ZDA sentence";
		return false;
	}

	_date = QDate(y, m, d);
	if (!_date.isValid()) {
		_errorString = "Invalid date";
		return false;
	}

	return true;
}

bool NMEAParser::loadFile(QFile *file)
{
	qint64 len;
	char line[80 + 2 + 1 + 1];
	bool nmea = false;


	_errorLine = 1;
	_errorString.clear();

	_GGA = false;
	_tracks.append(TrackData());

	while (!file->atEnd()) {
		len = file->readLine(line, sizeof(line));

		if (len < 0) {
			_errorString = "I/O error";
			return false;
		} else if (len > (qint64)sizeof(line) - 1) {
			_errorString = "Line limit exceeded";
			return false;
		}

		if (!validSentence(line, len)) {
			if (nmea)
				fprintf(stderr, "%s:%d: Invalid NMEA sentence\n",
				  qPrintable(file->fileName()), _errorLine);
			_errorLine++;
			continue;
		} else
			nmea = true;

		if (!memcmp(line + 3, "RMC,", 4)) {
			if (!readRMC(line + 7, len))
				return false;
		} else if (!memcmp(line + 3, "GGA,", 4)) {
			if (!readGGA(line + 7, len))
				return false;
		} else if (!memcmp(line + 3, "WPL,", 4)) {
			if (!readWPL(line + 7, len))
				return false;
		} else if (!memcmp(line + 3, "ZDA,", 4)) {
			if (!readZDA(line + 7, len))
				return false;
		}

		_errorLine++;
	}

	if (!_tracks.last().size() && !_waypoints.size()) {
		_errorString = "No usable NMEA sentence found";
		return false;
	}

	return true;
}
