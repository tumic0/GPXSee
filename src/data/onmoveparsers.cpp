#include <QFileInfo>
#include <QDir>
#include <QtEndian>
#include "onmoveparsers.h"


static inline quint16 u16(const char *buffer)
{
	return qFromLittleEndian<quint16>(buffer);
}

static inline qint16 s16(const char *buffer)
{
	return qFromLittleEndian<qint16>(buffer);
}

static inline qint32 s32(const char *buffer)
{
	return qFromLittleEndian<qint32>(buffer);
}


bool OMDParser::readHeaderFile(const QString &omdPath, Header &hdr)
{
	QFileInfo fi(omdPath);
	QString path = fi.absoluteDir().filePath(fi.baseName() + ".OMH");
	QFile file(path);
	char buffer[60];

	if (!file.open(QIODevice::ReadOnly)) {
		qWarning("%s: %s", qPrintable(path), qPrintable(file.errorString()));
		return false;
	}
	if (file.read(buffer, sizeof(buffer)) != sizeof(buffer)) {
		qWarning("%s: invalid OMH file", qPrintable(path));
		return false;
	}

	quint8 Y = buffer[14];
	quint8 M = buffer[15];
	quint8 D = buffer[16];
	quint8 h = buffer[17];
	quint8 m = buffer[18];
	quint16 ascent = u16(buffer + 22);
	quint16 descent = u16(buffer + 24);
	quint8 avgHr = buffer[12];
	quint8 maxHr = buffer[13];

	QDateTime date(QDate(Y + 2000, M, D), QTime(h, m), Qt::UTC);
	if (!date.isValid()) {
		qWarning("%s: invalid date", qPrintable(path));
		return false;
	}

	hdr.date = date;
	hdr.hr = avgHr || maxHr;
	hdr.elevation = ascent || descent;

	return true;
}

bool OMDParser::readF1(const char *chunk, const Header &hdr, Sequence &seq,
  SegmentData &segment)
{
	if (seq.cnt > 1) {
		_errorString = "invalid chunk sequence";
		return false;
	}

	qint32 lat = s32(chunk);
	qint32 lon = s32(chunk + 4);
	quint16 sec = u16(chunk + 12);
	quint8 fia = chunk[14];
	qint16 alt = s16(chunk + 15);

	if (fia == 3) {
		Trackpoint t(Coordinates(lon / 1000000.0, lat / 1000000.0));
		if (!t.coordinates().isValid()) {
			_errorString = "invalid coordinates";
			return false;
		}
		t.setTimestamp(QDateTime(hdr.date.date(),
		  hdr.date.time().addSecs(sec), Qt::UTC));
		if (hdr.elevation)
			t.setElevation(alt);

		seq.idx[seq.cnt] = segment.size();
		segment.append(t);
	} else
		seq.idx[seq.cnt] = -1;

	seq.cnt++;

	return true;
}

bool OMDParser::readF2(const char *chunk, const Header &hdr, Sequence &seq,
  SegmentData &segment)
{
	if (!seq.cnt) {
		_errorString = "invalid chunk sequence";
		return false;
	}

	quint16 speed1 = u16(chunk + 2);
	quint8 hr1 = chunk[6];
	quint16 speed2 = u16(chunk + 12);
	quint8 hr2 = chunk[16];

	if (seq.idx[0] >= 0) {
		Trackpoint &p0 = segment[seq.idx[0]];
		if (hdr.hr)
			p0.setHeartRate(hr1);
		p0.setSpeed(speed1 / 360.0);
	}
	if (seq.idx[1] >= 0) {
		Trackpoint &p1 = segment[seq.idx[1]];
		if (hdr.hr)
			p1.setHeartRate(hr2);
		p1.setSpeed(speed2 / 360.0);
	}

	seq.idx[0] = -1;
	seq.idx[1] = -1;
	seq.cnt = 0;

	return true;
}

bool OMDParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons, QVector<Waypoint> &waypoints)
{
	Q_UNUSED(routes);
	Q_UNUSED(waypoints);
	Q_UNUSED(polygons);
	SegmentData segment;
	Header hdr;
	Sequence seq;
	char chunk[20];
	qint64 size;

	// If no header file is found or it is invalid, continue with the default
	// header values. The track will have a fictional date and possibly some
	// zero-graphs, but it will be still usable.
	readHeaderFile(file->fileName(), hdr);

	while ((size = file->read(chunk, sizeof(chunk))) == sizeof(chunk)) {
		switch ((quint8)chunk[19]) {
			case 0xF1:
				if (!readF1(chunk, hdr, seq, segment))
					return false;
				break;
			case 0xF2:
				if (!readF2(chunk, hdr, seq, segment))
					return false;
				break;
			default:
				_errorString = "invalid chunk type";
				return false;
		}
	}

	if (size < 0) {
		_errorString = "I/O error";
		return false;
	} else if (size) {
		_errorString = "unexpected end of file";
		return false;
	}

	tracks.append(TrackData());
	tracks.last().append(segment);

	return true;
}


bool GHPParser::readHeaderFile(const QString &ghpPath, Header &hdr)
{
	QFileInfo fi(ghpPath);
	QString path = fi.absoluteDir().filePath(fi.baseName() + ".GHT");
	QFile file(path);
	char buffer[96];

	if (!file.open(QIODevice::ReadOnly)) {
		qWarning("%s: %s", qPrintable(path), qPrintable(file.errorString()));
		return false;
	}
	if (file.read(buffer, sizeof(buffer)) != sizeof(buffer)) {
		qWarning("%s: invalid GHT file", qPrintable(path));
		return false;
	}

	quint8 Y = buffer[0];
	quint8 M = buffer[1];
	quint8 D = buffer[2];
	quint8 h = buffer[3];
	quint8 m = buffer[4];
	quint8 s = buffer[5];
	quint8 avgHr = buffer[61];
	quint8 maxHr = buffer[60];

	QDateTime date(QDate(Y + 2000, M, D), QTime(h, m, s), Qt::UTC);
	if (!date.isValid()) {
		qWarning("%s: invalid date", qPrintable(path));
		return false;
	}

	hdr.date = date;
	hdr.hr = avgHr || maxHr;

	return true;
}

bool GHPParser::readF0(const char *chunk, const Header &hdr, int &time,
  SegmentData &segment)
{
	qint32 lat = s32(chunk);
	qint32 lon = s32(chunk + 4);
	qint16 alt = s16(chunk + 8);
	quint16 speed = u16(chunk + 10);
	quint8 hr = chunk[12];
	quint8 fia = chunk[13];
	quint8 ms = chunk[16];

	if (fia == 3) {
		Trackpoint t(Coordinates(lon / 1000000.0, lat / 1000000.0));
		if (!t.coordinates().isValid()) {
			_errorString = "invalid coordinates";
			return false;
		}
		t.setTimestamp(QDateTime(hdr.date.date(),
		  hdr.date.time().addMSecs(time * 100), Qt::UTC));
		t.setSpeed(speed / 360.0);
		t.setElevation(alt);
		if (hdr.hr)
			t.setHeartRate(hr);

		segment.append(t);
	}

	time += ms;

	return true;
}

bool GHPParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons, QVector<Waypoint> &waypoints)
{
	Q_UNUSED(routes);
	Q_UNUSED(waypoints);
	Q_UNUSED(polygons);
	SegmentData segment;
	Header hdr;
	int time = 0;
	char chunk[20];
	qint64 size;

	// see OMD
	readHeaderFile(file->fileName(), hdr);

	while ((size = file->read(chunk, sizeof(chunk))) == sizeof(chunk))
		if (!readF0(chunk, hdr, time, segment))
			return false;

	if (size < 0) {
		_errorString = "I/O error";
		return false;
	} else if (size) {
		_errorString = "unexpected end of file";
		return false;
	}

	tracks.append(TrackData());
	tracks.last().append(segment);

	return true;
}
