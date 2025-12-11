#include <QDataStream>
#include <QDateTime>
#include <QTimeZone>
#include <QRegularExpression>
#include <QUrl>
#include "common/util.h"
#include "mp4parser.h"

#define alignup(n, k) (((n) + (k) - 1) / (k) * (k))

static constexpr quint32 TAG(const char name[4])
{
	return (static_cast<quint32>(name[0]) << 24)
	  + (static_cast<quint32>(name[1]) << 16)
	  + (static_cast<quint32>(name[2]) << 8)
	  + (static_cast<quint32>(name[3]));
}

constexpr quint32 FTYP = TAG("ftyp");
constexpr quint32 MOOV = TAG("moov");
constexpr quint32 MVHD = TAG("mvhd");
constexpr quint32 TRAK = TAG("trak");
constexpr quint32 MDIA = TAG("mdia");
constexpr quint32 HDLR = TAG("hdlr");
constexpr quint32 MINF = TAG("minf");
constexpr quint32 MHLR = TAG("mhlr");
constexpr quint32 META = TAG("meta");
constexpr quint32 STBL = TAG("stbl");
constexpr quint32 STSD = TAG("stsd");
constexpr quint32 STSZ = TAG("stsz");
constexpr quint32 STCO = TAG("stco");
constexpr quint32 CO64 = TAG("co64");
constexpr quint32 GPMD = TAG("gpmd");
constexpr quint32 UDTA = TAG("udta");
constexpr quint32 XYZ = TAG("\xa9xyz");

constexpr quint32 GPS5 = TAG("GPS5");
constexpr quint32 GPS9 = TAG("GPS9");
constexpr quint32 GPSU = TAG("GPSU");
constexpr quint32 SCAL = TAG("SCAL");

static bool entry(QDataStream &stream, quint32 size, SegmentData &segment,
  bool &gps9, bool &gps5)
{
	quint32 key;
	quint8 ss, type;
	quint16 repeat;
	QDateTime base;
	QByteArray date(16, Qt::Initialization::Uninitialized);
	qint32 scale[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
	static const QDateTime dt2000 = QDateTime(QDate(2000, 1, 1), QTime(0, 0),
	  QTimeZone::utc());

	do {
		stream >> key >> type >> ss >> repeat;
		if (stream.status())
			return false;
		size -= 8;
		quint32 ps = alignup(ss * repeat, 4);

		if (!type) {
			if (!entry(stream, ps, segment, gps9, gps5))
				return false;
		} else {
			if (key == SCAL && type == 'l' && ss == 4
			  && (repeat == 5 || repeat == 9)) {
				for (quint16 i = 0; i < repeat; i++)
					stream >> scale[i];
				if (stream.status())
					return false;
			} else if (!gps5 && key == GPS9 && type == '?' && ss == 32) {
				qint32 lat, lon, alt, spd2, spd3, days, secs;
				quint16 dop, fix;

				for (quint16 i = 0; i < repeat; i++) {
					stream >> lat >> lon >> alt >> spd2 >> spd3 >> days >> secs
					  >> dop >> fix;
					if (stream.status())
						return false;
					Trackpoint t(Coordinates(lon / (double)scale[1],
					  lat / (double)scale[0]));
					QDateTime ts = dt2000.addDays(days)
					  .addMSecs(qRound((secs / (double)scale[6]) * 1000));
					t.setTimestamp(ts);
					t.setElevation(alt / (double)scale[2]);
					t.setSpeed(spd2 / (double)scale[3]);
					if (t.coordinates().isValid())
						segment.append(t);
					else
						return false;
				}
				gps9 = true;
			} else if (!gps9 && key == GPS5 && type == 'l' && ss == 20) {
				qint64 ms = qRound((1.0 / (double)repeat) * 1000);

				qint32 lat, lon, alt, spd2, spd3;
				for (quint16 i = 0; i < repeat; i++) {
					stream >> lat >> lon >> alt >> spd2 >> spd3;
					if (stream.status())
						return false;
					Trackpoint t(Coordinates(lon / (double)scale[1],
					  lat / (double)scale[0]));
					t.setTimestamp(base.addMSecs(ms * i));
					t.setElevation(alt / (double)scale[2]);
					t.setSpeed(spd2 / (double)scale[3]);
					if (t.coordinates().isValid())
						segment.append(t);
					else
						return false;
				}
				gps5 = true;
			} else if (!gps9 && key == GPSU && type == 'U' && ss == 16
			  && repeat == 1) {
				if (stream.readRawData(date.data(), date.size()) != date.size())
					return false;
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
				base = QDateTime::fromString(date, "yyMMddHHmmss.zzz")
				  .addYears(100);
#else // QT 6.7
				base = QDateTime::fromString(date, "yyMMddHHmmss.zzz", 2000);
#endif // QT 6.7
				base.setTimeZone(QTimeZone::utc());
			} else {
				if (stream.skipRawData(ps) != (qint64)ps)
					return false;
			}
		}
		size -= ps;
	} while (size);

	return true;
}

static bool hdr(QDataStream &stream, quint32 &type, quint64 &size,
  quint32 &hdrSize)
{
	quint32 size32;

	stream >> size32 >> type;
	if (stream.status())
		return false;
	if (size32 == 1) {
		hdrSize = 16;
		stream >> size;
		if (stream.status())
			return false;
	} else {
		hdrSize = 8;
		size = size32;
	}

	return (size >= hdrSize);
}

static bool dec(quint64 &atomSize, quint64 size)
{
	if (atomSize) {
		if (size > atomSize)
			return false;
		else
			atomSize -= size;
	}

	return true;
}

static bool ftyp(QDataStream &stream)
{
	quint32 type, hdrSize;
	quint64 size;

	if (!hdr(stream, type, size, hdrSize))
		return false;
	if (type != FTYP)
		return false;

	return (stream.skipRawData(size - hdrSize) == (qint64)(size - hdrSize));
}

static bool stsd(QDataStream &stream, quint64 atomSize, bool &gpmd)
{
	if (atomSize && atomSize < 8)
		return false;

	quint32 vf, num;
	stream >> vf >> num;
	if (stream.status())
		return false;

	for (quint32 i = 0; i < num; i++) {
		quint32 ds, format;
		stream >> ds >> format;
		stream.skipRawData(ds - 8);
		if (stream.status())
			return false;
		if (format == GPMD)
			gpmd = true;
	}

	return true;
}

static bool stsz(QDataStream &stream, quint64 atomSize, QVector<quint32> &sizes)
{
	if (atomSize && atomSize < 12)
		return false;

	quint32 vf, ss, num;
	stream >> vf >> ss >> num;
	if (stream.status())
		return false;

	if (ss)
		sizes = QVector<quint32>(num, ss);
	else {
		sizes.resize(num);
		for (quint32 i = 0; i < num; i++)
			stream >> sizes[i];
		if (stream.status())
			return false;
	}

	return true;
}

static bool stco(QDataStream &stream, quint64 atomSize, QVector<quint64> &chunks)
{
	if (atomSize && atomSize < 8)
		return false;

	quint32 vf, num, chunk;
	stream >> vf >> num;
	if (stream.status())
		return false;

	chunks.resize(num);
	for (quint32 i = 0; i < num; i++) {
		stream >> chunk;
		chunks[i] = chunk;
	}
	if (stream.status())
		return false;

	return true;
}

static bool co64(QDataStream &stream, quint64 atomSize, QVector<quint64> &chunks)
{
	if (atomSize && atomSize < 8)
		return false;

	quint32 vf, num;
	stream >> vf >> num;
	if (stream.status())
		return false;

	chunks.resize(num);
	for (quint32 i = 0; i < num; i++)
		stream >> chunks[i];
	if (stream.status())
		return false;

	return true;
}

static bool stbl(QDataStream &stream, quint64 atomSize, QVector<quint32> &sizes,
  QVector<quint64> &chunks)
{
	quint32 type, hdrSize;
	quint64 size;
	bool gpmd = false;

	do {
		if (!(hdr(stream, type, size, hdrSize) && dec(atomSize, size)))
			return false;

		if (type == STSD) {
			if (!stsd(stream, size ? size - hdrSize : 0, gpmd))
				return false;
		} else if (type == STSZ && gpmd) {
			if (!stsz(stream, size ? size - hdrSize : 0, sizes))
				return false;
		} else if (type == STCO && gpmd) {
			if (!stco(stream, size ? size - hdrSize : 0, chunks))
				return false;
		} else if (type == CO64 && gpmd) {
			if (!co64(stream, size ? size - hdrSize : 0, chunks))
				return false;
		} else {
			if (size) {
				if (stream.skipRawData(size - hdrSize)
				  != (qint64)(size - hdrSize))
					return false;
			} else
				break;
		}
		if (stream.atEnd())
			break;
	} while (atomSize);

	if (chunks.size() != sizes.size())
		return false;

	return (!atomSize);
}

static bool hdlr(QDataStream &stream, quint64 atomSize, bool &mhlr)
{
	// HDLR atom size can not be zero
	if (atomSize < 24)
		return false;

	quint32 vf, type, subtype, manufacturer, flags, mask;
	stream >> vf >> type >> subtype >> manufacturer >> flags >> mask;
	if (stream.status()
	  || stream.skipRawData(atomSize - 24) != (qint64)atomSize - 24)
		return false;

	mhlr = (type == MHLR && subtype == META);

	return true;
}

static bool minf(QDataStream &stream, quint64 atomSize, QVector<quint32> &sizes,
  QVector<quint64> &chunks)
{
	quint32 type, hdrSize;
	quint64 size;

	do {
		if (!(hdr(stream, type, size, hdrSize) && dec(atomSize, size)))
			return false;

		if (type == STBL) {
			if (!stbl(stream, size ? size - hdrSize : 0, sizes, chunks))
				return false;
		} else {
			if (size) {
				if (stream.skipRawData(size - hdrSize)
				  != (qint64)(size - hdrSize))
					return false;
			} else
				break;
		}
		if (stream.atEnd())
			break;
	} while (atomSize);

	return (!atomSize);
}

static bool mdia(QDataStream &stream, quint64 atomSize, QVector<quint32> &sizes,
  QVector<quint64> &chunks)
{
	quint32 type, hdrSize;
	quint64 size;
	bool mhlr = false;

	do {
		if (!(hdr(stream, type, size, hdrSize) && dec(atomSize, size)))
			return false;

		if (type == HDLR) {
			if (!hdlr(stream, size ? size - hdrSize : 0, mhlr))
				return false;
		} else if (type == MINF && mhlr) {
			if (!minf(stream, size ? size - hdrSize : 0, sizes, chunks))
				return false;
		} else {
			if (size) {
				if (stream.skipRawData(size - hdrSize)
				  != (qint64)(size - hdrSize))
					return false;
			} else
				break;
		}
		if (stream.atEnd())
			break;
	} while (atomSize);

	return (!atomSize);
}

static bool trak(QDataStream &stream, quint64 atomSize, QVector<quint32> &sizes,
  QVector<quint64> &chunks)
{
	quint32 type, hdrSize;
	quint64 size;

	do {
		if (!(hdr(stream, type, size, hdrSize) && dec(atomSize, size)))
			return false;

		if (type == MDIA) {
			if (!mdia(stream, size ? size - hdrSize : 0, sizes, chunks))
				return false;
		} else {
			if (size) {
				if (stream.skipRawData(size - hdrSize)
				  != (qint64)(size - hdrSize))
					return false;
			} else
				break;
		}
		if (stream.atEnd())
			break;
	} while (atomSize);

	return (!atomSize);
}

static bool iso6709(const QByteArray &ba, Waypoint &wpt)
{
	static const QRegularExpression dd(
	  "^([-+]\\d{1,2}(?:\\.\\d*)?)([-+]\\d{1,3}(?:\\.\\d*)?)([-+]\\d+(?:\\.\\d*)?)?");
	static const QRegularExpression dm(
	  "^([-+])(\\d{2})(\\d{2}(?:\\.\\d*)?)([-+])(\\d{3})(\\d{2}(?:\\.\\d*)?)([-+]\\d+(?:\\.\\d*)?)?");
	static const QRegularExpression dms(
	  "^([-+])(\\d{2})(\\d{2})(\\d{2}(?:\\.\\d*)?)([-+])(\\d{3})(\\d{2})(\\d{2}(?:\\.\\d*)?)([-+]\\d+(?:\\.\\d*)?)?");
	bool ok;
	double ele;

	QRegularExpressionMatch ddMatch = dd.match(ba);
	if (ddMatch.hasMatch()) {
		wpt.setCoordinates(Coordinates(ddMatch.captured(2).toDouble(),
		  ddMatch.captured(1).toDouble()));
		ele = ddMatch.captured(3).toDouble(&ok);
		if (ok)
			wpt.setElevation(ele);
		return true;
	}
	QRegularExpressionMatch dmMatch = dm.match(ba);
	if (dmMatch.hasMatch()) {
		double lat = dmMatch.captured(2).toDouble()
		  + dmMatch.captured(3).toDouble() / 60;
		if (dmMatch.captured(1) == "-")
			lat = -lat;
		double lon = dmMatch.captured(5).toDouble()
		  + dmMatch.captured(6).toDouble() / 60;
		if (dmMatch.captured(4) == "-")
			lon = -lon;
		wpt.setCoordinates(Coordinates(lon, lat));
		ele = dmMatch.captured(7).toDouble(&ok);
		if (ok)
			wpt.setElevation(ele);
		return true;
	}
	QRegularExpressionMatch dmsMatch = dms.match(ba);
	if (dmsMatch.hasMatch()) {
		double lat = dmsMatch.captured(2).toDouble()
		  + dmsMatch.captured(3).toDouble() / 60
		  + dmsMatch.captured(4).toDouble() / 3600;
		if (dmsMatch.captured(1) == "-")
			lat = -lat;
		double lon = dmsMatch.captured(6).toDouble()
		  + dmsMatch.captured(7).toDouble() / 60
		  + dmsMatch.captured(8).toDouble() / 3600;
		if (dmsMatch.captured(5) == "-")
			lon = -lon;
		wpt.setCoordinates(Coordinates(lon, lat));
		ele = dmsMatch.captured(9).toDouble(&ok);
		if (ok)
			wpt.setElevation(ele);
		return true;
	}

	return false;
}

static bool xyz(QDataStream &stream, quint64 atomSize, Waypoint &wpt)
{
	if (atomSize && atomSize < 4)
		return false;

	quint16 size, lang;
	stream >> size >> lang;
	if (stream.status())
		return false;

	QByteArray ba(size, Qt::Initialization::Uninitialized);
	if (stream.readRawData(ba.data(), ba.size()) != ba.size())
		return false;

	if (!iso6709(ba, wpt))
		qWarning("%s: %s: invalid ISO6709 location",
		  qUtf8Printable(qobject_cast<QFile*>(stream.device())->fileName()),
		  qUtf8Printable(ba));

	return true;
}

static bool udta(QDataStream &stream, quint64 atomSize, Waypoint &wpt)
{
	quint32 type, hdrSize;
	quint64 size;

	do {
		if (!(hdr(stream, type, size, hdrSize) && dec(atomSize, size)))
			return false;

		if (type == XYZ) {
			if (!xyz(stream, size ? size - hdrSize : 0, wpt))
				return false;
		} else {
			if (size) {
				if (stream.skipRawData(size - hdrSize)
				  != (qint64)(size - hdrSize))
					return false;
			} else
				break;
		}
		if (stream.atEnd())
			break;
	} while (atomSize);

	return (!atomSize);
}

static bool mvhd(QDataStream &stream, quint64 atomSize, Waypoint &wpt)
{
	static const QDateTime dt1904 = QDateTime(QDate(1904, 1, 1), QTime(0, 0),
	  QTimeZone::utc());

	if (atomSize && atomSize < 8)
		return false;

	quint32 vf, time;
	stream >> vf >> time;
	if (stream.status())
		return false;

	wpt.setTimestamp(dt1904.addSecs(time));

	return (stream.skipRawData(atomSize - 8) == (qint64)(atomSize - 8));
}

static bool moov(QDataStream &stream, quint64 atomSize, QVector<quint32> &sizes,
  QVector<quint64> &chunks, Waypoint &wpt)
{
	quint32 type, hdrSize;
	quint64 size;

	do {
		if (!(hdr(stream, type, size, hdrSize) && dec(atomSize, size)))
			return false;

		if (type == TRAK) {
			if (!trak(stream, size ? size - hdrSize : 0, sizes, chunks))
				return false;
		} else if (type == MVHD) {
			if (!mvhd(stream, size ? size - hdrSize : 0, wpt))
				return false;
		} else if (type == UDTA) {
			if (!udta(stream, size ? size - hdrSize : 0, wpt))
				return false;
		} else {
			if (size) {
				if (stream.skipRawData(size - hdrSize)
				  != (qint64)(size - hdrSize))
					return false;
			} else
				break;
		}
		if (stream.atEnd())
			break;
	} while (atomSize);

	return (!atomSize);
}

static bool atoms(QDataStream &stream, QVector<quint32> &sizes,
  QVector<quint64> &chunks, Waypoint &wpt)
{
	quint32 type, hdrSize;
	quint64 size;

	do {
		if (!hdr(stream, type, size, hdrSize))
			return false;

		if (type == MOOV) {
			if (!moov(stream, size ? size - hdrSize : 0, sizes, chunks, wpt))
				return false;
		} else {
			if (size) {
				if (stream.skipRawData(size - hdrSize)
				  != (qint64)(size - hdrSize))
					return false;
			} else
				break;
		}
	} while (!stream.atEnd());

	return true;
}

bool MP4Parser::mp4(QFile *file, QVector<quint32> &sizes,
  QVector<quint64> &chunks, Waypoint &wpt)
{
	QDataStream stream(file);

	stream.setByteOrder(QDataStream::BigEndian);

	if (!ftyp(stream)) {
		_errorString = "Not a MP4 file";
		return false;
	}

	if (!atoms(stream, sizes, chunks, wpt)) {
		_errorString = "MP4 file format error";
		return false;
	}

	return true;
}

bool MP4Parser::gpmf(QFile *file, quint64 offset, quint32 size,
  SegmentData &segment)
{
	char magic[4];
	bool gps9 = false, gps5 = false;

	if (!file->seek(offset)) {
		_errorString = "Invalid GPMF chunk offset";
		return false;
	}
	if ((file->peek(magic, sizeof(magic)) != sizeof(magic))
	  || memcmp(magic, "DEVC", sizeof(magic))) {
		_errorString = "Invalid GPMF file";
		return false;
	}

	QDataStream stream(file);
	stream.setByteOrder(QDataStream::BigEndian);

	if (!entry(stream, size, segment, gps9, gps5)) {
		_errorString = "GPMF parse error";
		return false;
	}

	return true;
}

bool MP4Parser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons, QVector<Waypoint> &waypoints)
{
	Q_UNUSED(routes);
	Q_UNUSED(polygons);
	QVector<quint32> sizes;
	QVector<quint64> chunks;
	SegmentData segment;
	Waypoint wpt;

	if (!mp4(file, sizes, chunks, wpt)) {
		QString es(_errorString);

		if (gpmf(file, 0, file->size(), segment)) {
			if (segment.size()) {
				tracks.append(segment);
				tracks.last().setFile(file->fileName());
				return true;
			} else
				_errorString = "No GPS data found in GPMF";
		} else
			_errorString = es;
	} else {
		if (chunks.size()) {
			for (int i = 0; i < chunks.size(); i++)
				if (!gpmf(file, chunks.at(i), sizes.at(i), segment))
					return false;
			if (segment.size()) {
				TrackData t(segment);
				t.setFile(file->fileName());
				t.markVideo(true);
				tracks.append(t);
				return true;
			}
		}
		if (wpt.coordinates().isValid()) {
			wpt.setName(Util::file2name(file->fileName()));
			wpt.setFile(file->fileName());
			waypoints.append(wpt);
			return true;
		}

		_errorString = "No GPS data found in MP4";
	}

	return false;
}
