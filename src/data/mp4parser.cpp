#include <QtMath>
#include <QtEndian>
#include <QDataStream>
#include <QDateTime>
#include <QTimeZone>
#include <QRegularExpression>
#include <QBuffer>
#include <QJsonDocument>
#include <QJsonObject>
#include "common/util.h"
#include "nmeaparser.h"
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
constexpr quint32 META = TAG("meta");
constexpr quint32 STBL = TAG("stbl");
constexpr quint32 STSD = TAG("stsd");
constexpr quint32 STSC = TAG("stsc");
constexpr quint32 STSZ = TAG("stsz");
constexpr quint32 STCO = TAG("stco");
constexpr quint32 CO64 = TAG("co64");
constexpr quint32 GPMD = TAG("gpmd");
constexpr quint32 RTMD = TAG("rtmd");
constexpr quint32 CAMM = TAG("camm");
constexpr quint32 UDTA = TAG("udta");
constexpr quint32 XYZ = TAG("\xa9xyz");
constexpr quint32 GPS = TAG("gps ");
constexpr quint32 FREE = TAG("free");
constexpr quint32 CGPS = TAG("GPS ");

constexpr quint32 GPS5 = TAG("GPS5");
constexpr quint32 GPS9 = TAG("GPS9");
constexpr quint32 GPSU = TAG("GPSU");
constexpr quint32 SCAL = TAG("SCAL");

static bool gpmfEntry(QDataStream &stream, quint32 size, SegmentData &segment,
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
			if (!gpmfEntry(stream, ps, segment, gps9, gps5))
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

static bool coord(QDataStream &stream, quint16 len, double &val)
{
	if (len != 24)
		return false;

	quint32 deg, degSc, min, minSc, sec, secSc;
	stream >> deg >> degSc >> min >> minSc >> sec >> secSc;
	if (stream.status())
		return false;

	val = (deg / (double)degSc) + (min / (double)minSc)/60.0
	  + (sec / (double)secSc)/3600.0;

	return true;
}

static bool latRef(QDataStream &stream, quint16 len, bool &neg)
{
	if (len != 1)
		return false;

	quint8 ref;
	stream >> ref;
	if (stream.status())
		return false;

	if (ref == 'S') {
		neg = true;
		return true;
	} else if (ref == 'N') {
		neg = false;
		return true;
	} else
		return false;
}

static bool lonRef(QDataStream &stream, quint16 len, bool &neg)
{
	if (len != 1)
		return false;

	quint8 ref;
	stream >> ref;
	if (stream.status())
		return false;

	if (ref == 'W') {
		neg = true;
		return true;
	} else if (ref == 'E') {
		neg = false;
		return true;
	} else
		return false;
}

static bool time(QDataStream &stream, quint16 len, QTime &t)
{
	if (len != 24)
		return false;

	quint32 h, hSc, m, mSc, s, sSc;
	stream >> h >> hSc >> m >> mSc >> s >> sSc;
	if (stream.status() || hSc != 1 || mSc != 1)
		return false;

	double ds = s / (double)sSc;
	int sec = qFloor(ds);
	int ms = qRound((ds - sec) * 1000.0);

	t = QTime(h, m, sec, ms);

	return true;
}

static bool date(QDataStream &stream, quint16 len, QDate &d)
{
	QByteArray ba(len, Qt::Initialization::Uninitialized);
	if (stream.readRawData(ba.data(), ba.size()) != ba.size())
		return false;

	d = QDate::fromString(ba, "yyyy:MM:dd");

	return true;
}

static bool rtmfEntry(QDataStream &stream, quint32 size, SegmentData &segment)
{
	quint16 len, id;
	double lon = NAN, lat = NAN;
	bool lonr = false, latr = false;
	QTime t;
	QDate d;

	do {
		if (size < 4)
			return false;
		stream >> id >> len;
		if (stream.status())
			return false;
		size -= 4;

		if (id == 0x8300)
			continue;
		if (id == 0x060e)
			len = 12;

		switch (id) {
			case 0x8501:
				if (!latRef(stream, len, latr))
					return false;
				break;
			case 0x8502:
				if (!coord(stream, len, lat))
					return false;
				break;
			case 0x8503:
				if (!lonRef(stream, len, lonr))
					return false;
				break;
			case 0x8504:
				if (!coord(stream, len, lon))
					return false;
				break;
			case 0x8507:
				if (!time(stream, len, t))
					return false;
				break;
			case 0x851d:
				if (!date(stream, len, d))
					return false;
				break;
			default:
				if (size < len || stream.skipRawData(len) != len)
					return false;
		}
		size -= len;
	} while (size);

	if (lonr)
		lon = -lon;
	if (latr)
		lat = -lat;

	Trackpoint tp(Coordinates(lon, lat));
	tp.setTimestamp(QDateTime(d, t, QTimeZone::utc()));
	if (tp.coordinates().isValid())
		segment.append(tp);

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

bool MP4Parser::stsd(QDataStream &stream, quint64 atomSize, Format &format,
  quint32 &id)
{
	if (atomSize < 8)
		return false;

	quint32 vf, num;
	stream >> vf >> num;
	if (stream.status())
		return false;
	atomSize -= 8;

	for (quint32 i = 0; i < num; i++) {
		quint32 ds, fmt;

		if (atomSize < 8)
			return false;
		stream >> ds >> fmt;
		if (atomSize < ds)
			return false;
		stream.skipRawData(ds - 8);
		if (stream.status())
			return false;
		switch (fmt) {
			case GPMD:
				format = GPMDFormat;
				id = i + 1;
				break;
			case RTMD:
				format = RTMDFormat;
				id = i + 1;
				break;
			case CAMM:
				format = CAMMFormat;
				id = i + 1;
				break;
			default:
				format = UnknownFormat;
		}

		atomSize -= ds;
	}

	return (atomSize)
	  ? (stream.skipRawData(atomSize) == (qint64)atomSize) : true;
}

static bool stsz(QDataStream &stream, quint64 atomSize, QVector<quint32> &sizes)
{
	if (atomSize < 12)
		return false;

	quint32 vf, ss, num;
	stream >> vf >> ss >> num;
	if (stream.status())
		return false;
	atomSize -= 12;

	if (ss)
		sizes = QVector<quint32>(num, ss);
	else {
		if (atomSize < num * 4)
			return false;

		sizes.resize(num);
		for (quint32 i = 0; i < num; i++)
			stream >> sizes[i];
		if (stream.status())
			return false;
		atomSize -= num * 4;
	}

	return (atomSize)
	  ? (stream.skipRawData(atomSize) == (qint64)atomSize) : true;
}

bool MP4Parser::stsc(QDataStream &stream, quint64 atomSize,
  QVector<Table> &tables)
{
	if (atomSize < 12)
		return false;

	quint32 vf, num;
	stream >> vf >> num;
	if (stream.status())
		return false;
	atomSize -= 8;

	if (atomSize < num * 12)
		return false;

	tables.resize(num);
	for (quint32 i = 0; i < num; i++)
		stream >> tables[i].first >> tables[i].samples >> tables[i].id;
	if (stream.status())
		return false;
	atomSize -= num * 12;

	return (atomSize)
	  ? (stream.skipRawData(atomSize) == (qint64)atomSize) : true;
}

static bool stco(QDataStream &stream, quint64 atomSize, QVector<quint64> &chunks)
{
	if (atomSize < 8)
		return false;

	quint32 vf, num, chunk;
	stream >> vf >> num;
	if (stream.status())
		return false;
	atomSize -= 8;

	if (atomSize < num * 4)
		return false;

	chunks.resize(num);
	for (quint32 i = 0; i < num; i++) {
		stream >> chunk;
		chunks[i] = chunk;
	}
	if (stream.status())
		return false;
	atomSize -= num * 4;

	return (atomSize)
	  ? (stream.skipRawData(atomSize) == (qint64)atomSize) : true;
}

static bool co64(QDataStream &stream, quint64 atomSize, QVector<quint64> &chunks)
{
	if (atomSize < 8)
		return false;

	quint32 vf, num;
	stream >> vf >> num;
	if (stream.status())
		return false;
	atomSize -= 8;

	if (atomSize < num * 8)
		return false;

	chunks.resize(num);
	for (quint32 i = 0; i < num; i++)
		stream >> chunks[i];
	if (stream.status())
		return false;
	atomSize -= num * 8;

	return (atomSize)
	  ? (stream.skipRawData(atomSize) == (qint64)atomSize) : true;
}

bool MP4Parser::stbl(QDataStream &stream, quint64 atomSize, Metadata &meta)
{
	quint32 type, hdrSize;
	quint64 size;

	do {
		if (!(hdr(stream, type, size, hdrSize) && dec(atomSize, size)))
			return false;

		if (type == STSD) {
			if (!stsd(stream, size ? size - hdrSize : 0, meta.format, meta.id))
				return false;
		} else if (type == STSC && meta.format) {
			if (!stsc(stream, size ? size - hdrSize : 0, meta.tables))
				return false;
		} else if (type == STSZ && meta.format) {
			if (!stsz(stream, size ? size - hdrSize : 0, meta.sizes))
				return false;
		} else if (type == STCO && meta.format) {
			if (!stco(stream, size ? size - hdrSize : 0, meta.chunks))
				return false;
		} else if (type == CO64 && meta.format) {
			if (!co64(stream, size ? size - hdrSize : 0, meta.chunks))
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

static bool hdlr(QDataStream &stream, quint64 atomSize, bool &meta)
{
	if (atomSize < 24)
		return false;

	quint32 vf, type, subtype, manufacturer, flags, mask;
	stream >> vf >> type >> subtype >> manufacturer >> flags >> mask;
	if (stream.status()
	  || stream.skipRawData(atomSize - 24) != (qint64)atomSize - 24)
		return false;

	meta = (subtype == META || subtype == CAMM);

	return true;
}

bool MP4Parser::minf(QDataStream &stream, quint64 atomSize, Metadata &meta)
{
	quint32 type, hdrSize;
	quint64 size;

	do {
		if (!(hdr(stream, type, size, hdrSize) && dec(atomSize, size)))
			return false;

		if (type == STBL) {
			if (!stbl(stream, size ? size - hdrSize : 0, meta))
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

bool MP4Parser::mdia(QDataStream &stream, quint64 atomSize, Metadata &meta)
{
	quint32 type, hdrSize;
	quint64 size;
	bool ismeta = false;

	do {
		if (!(hdr(stream, type, size, hdrSize) && dec(atomSize, size)))
			return false;

		if (type == HDLR) {
			if (!hdlr(stream, size ? size - hdrSize : 0, ismeta))
				return false;
		} else if (type == MINF && ismeta) {
			if (!minf(stream, size ? size - hdrSize : 0, meta))
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

bool MP4Parser::trak(QDataStream &stream, quint64 atomSize, Metadata &meta)
{
	quint32 type, hdrSize;
	quint64 size;

	do {
		if (!(hdr(stream, type, size, hdrSize) && dec(atomSize, size)))
			return false;

		if (type == MDIA) {
			if (!mdia(stream, size ? size - hdrSize : 0, meta))
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
	  "^([-+]\\d{1,2}(?:\\.\\d*)?)([-+]\\d{1,3}(?:\\.\\d*)?)([-+]\\d+"
	  "(?:\\.\\d*)?)?"
	);
	static const QRegularExpression dm(
	  "^([-+])(\\d{2})(\\d{2}(?:\\.\\d*)?)([-+])(\\d{3})(\\d{2}(?:\\.\\d*)?)"
	  "([-+]\\d+(?:\\.\\d*)?)?"
	);
	static const QRegularExpression dms(
	  "^([-+])(\\d{2})(\\d{2})(\\d{2}(?:\\.\\d*)?)([-+])(\\d{3})(\\d{2})"
	  "(\\d{2}(?:\\.\\d*)?)([-+]\\d+(?:\\.\\d*)?)?"
	);
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

static bool udtam(QDataStream &stream, quint64 atomSize, Waypoint &wpt)
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

bool MP4Parser::gps(QDataStream &stream, quint64 atomSize, Metadata &meta)
{
	if (atomSize < 8)
		return false;

	meta.format = NovatekFormat;
	meta.id = 1;
	meta.tables.append(Table(1, 1, 1));

	quint64 hdr;
	quint32 offset, size;

	stream >> hdr;
	if (stream.status())
		return false;
	atomSize -= 8;
	while (atomSize >= 8) {
		stream >> offset >> size;
		if (stream.status())
			return false;
		atomSize -= 8;

		meta.sizes.append(size);
		meta.chunks.append(offset);
	}

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

bool MP4Parser::moov(QDataStream &stream, quint64 atomSize, Metadata &meta,
  Waypoint &wpt)
{
	quint32 type, hdrSize;
	quint64 size;

	do {
		if (!(hdr(stream, type, size, hdrSize) && dec(atomSize, size)))
			return false;

		if (!meta.format && type == TRAK) {
			if (!trak(stream, size ? size - hdrSize : 0, meta))
				return false;
		} else if (type == MVHD) {
			if (!mvhd(stream, size ? size - hdrSize : 0, wpt))
				return false;
		} else if (type == UDTA) {
			if (!udtam(stream, size ? size - hdrSize : 0, wpt))
				return false;
		} else if (type == GPS) {
			if (!gps(stream, size ? size - hdrSize : 0, meta))
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

bool MP4Parser::udta(QDataStream &stream, quint64 atomSize, Metadata &meta)
{
	qint64 offset = stream.device()->pos();

	if (atomSize < 24)
		return (stream.skipRawData(atomSize) == (qint64)(atomSize));

	QByteArray magic(16, Qt::Initialization::Uninitialized);
	if (stream.skipRawData(8) != 8)
		return false;
	if (stream.readRawData(magic.data(), magic.size()) != magic.size())
		return false;

	if (magic == "__V35AX_QVDATA__") {
		meta.format = LigoJSONFormat;
		meta.id = 1;
		meta.tables.append(Table(meta.tables.size() + 1, 1, 1));
		meta.chunks.append(offset);
		meta.sizes.append((quint32)atomSize);
	}

	return (stream.skipRawData(atomSize - 24) == (qint64)(atomSize - 24));
}

bool MP4Parser::gpsf(QDataStream &stream, quint64 atomSize, Metadata &meta)
{
	qint64 offset = stream.device()->pos();

	meta.format = PittasoftFormat;
	meta.id = 1;
	meta.tables.append(Table(meta.tables.size() + 1, 1, 1));
	meta.chunks.append(offset);
	meta.sizes.append((quint32)atomSize);

	return (stream.skipRawData(atomSize) == (qint64)(atomSize));
}

bool MP4Parser::free2(QDataStream &stream, quint64 atomSize, Metadata &meta)
{
	quint32 type, hdrSize;
	quint64 size;

	do {
		if (!(hdr(stream, type, size, hdrSize) && dec(atomSize, size)))
			return false;

		if (!meta.format && type == GPS) {
			if (!gpsf(stream, size ? size - hdrSize : 0, meta))
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

bool MP4Parser::free(QDataStream &stream, quint64 atomSize, Metadata &meta)
{
	qint64 pos = stream.device()->pos();
	if (free2(stream, atomSize, meta))
		return true;
	else
		return stream.device()->seek(pos + atomSize);
}

bool MP4Parser::atoms(QDataStream &stream, Metadata &meta, Waypoint &wpt)
{
	quint32 type, hdrSize;
	quint64 size;

	do {
		if (!hdr(stream, type, size, hdrSize))
			return false;
		if (type == MOOV) {
			if (!moov(stream, size ? size - hdrSize : 0, meta, wpt))
				return false;
		} else if (type == UDTA) {
			if (!udta(stream, size ? size - hdrSize : 0, meta))
				return false;
		} else if (type == FREE) {
			if (!free(stream, size ? size - hdrSize : 0, meta))
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

bool MP4Parser::mp4(QFile *file, Metadata &meta, Waypoint &wpt)
{
	QDataStream stream(file);

	stream.setByteOrder(QDataStream::BigEndian);

	if (!ftyp(stream)) {
		_errorString = "Not a MP4 file";
		return false;
	}

	if (!atoms(stream, meta, wpt)) {
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
		_errorString = "Invalid GPMF sample offset";
		return false;
	}
	if ((file->peek(magic, sizeof(magic)) != sizeof(magic))
	  || memcmp(magic, "DEVC", sizeof(magic))) {
		_errorString = "Invalid GPMF data";
		return false;
	}

	QDataStream stream(file);
	stream.setByteOrder(QDataStream::BigEndian);

	if (!gpmfEntry(stream, size, segment, gps9, gps5)) {
		_errorString = "GPMF parse error";
		return false;
	}

	return true;
}

bool MP4Parser::rtmf(QFile *file, quint64 offset, quint32 size,
  SegmentData &segment)
{
	if (!file->seek(offset)) {
		_errorString = "Invalid RTMF sample offset";
		return false;
	}

	QDataStream stream(file);
	stream.setByteOrder(QDataStream::BigEndian);

	quint16 hdrLen;
	stream >> hdrLen;
	if (stream.status() || hdrLen > size
	  || stream.skipRawData(hdrLen - 2) != (hdrLen - 2)) {
		_errorString = "Invalid RTMF data";
		return false;
	}
	size -= hdrLen;

	if (!rtmfEntry(stream, size, segment)) {
		_errorString = "RTMF parse error";
		return false;
	}

	return true;
}

bool MP4Parser::camm(QFile *file, quint64 offset, quint32 size,
  SegmentData &segment)
{
	if (!file->seek(offset)) {
		_errorString = "Invalid CAMM sample offset";
		return false;
	}

	QDataStream stream(file);
	stream.setByteOrder(QDataStream::LittleEndian);

	quint16 reserved, type;
	stream >> reserved >> type;
	if (stream.status() || size < 4) {
		_errorString = "Invalid CAMM data";
		return false;
	}
	size -= 4;

	unsigned len = 0;
	double time = NAN, lon = NAN, lat = NAN, ele = NAN;
	float elef;
	int fix;
	static const QDateTime epoch(QDate(1980, 1, 6), QTime(0, 0),
	  QTimeZone::utc());

	switch (type) {
		case 5:
			stream >> lat >> lon >> ele;
			len = 24;
			break;
		case 6:
			stream >> time >> fix >> lat >> lon;
			stream.readRawData((char*)&elef, sizeof(elef));
			ele = elef;
			len = 56;
			break;
		default:
			return true;
	}

	if (stream.status() || size < len) {
		_errorString = "CAMM parse error";
		return false;
	}

	Trackpoint t(Coordinates(lon, lat));
	t.setElevation(ele);
	if (!std::isnan(time)) {
		qint64 msec = (qint64)(time * 1000);
		t.setTimestamp(epoch.addMSecs(msec));
	}
	if (t.coordinates().isValid())
		segment.append(t);

	return true;
}

template<typename T>
static double lon2dd(T dm, quint8 ref)
{
	int deg = (dm / 100.0);
	T min = dm - (T)(deg * 100);
	return (ref == 'W')
	  ? -((double)deg + (double)min / 60.0) : (double)deg + (double)min / 60.0;
}

template<typename T>
static double lat2dd(T dm, quint8 ref)
{
	int deg = (dm / 100.0);
	T min = dm - (T)(deg * 100);
	return (ref == 'S')
	  ? -((double)deg + (double)min / 60.0) : (double)deg + (double)min / 60.0;
}

static int novatekOffset(const QByteArray &ba)
{
	int state = 0;

	for (int i = 24; i < ba.size() - 13; i++) {
		char c = ba.at(i);

		switch (state) {
			case 0:
				if (c == 'A')
					state = 1;
				break;
			case 1:
				if (c == 'N' || c == 'S')
					state = 2;
				else
					state = 0;
				break;
			case 2:
				if (c == 'E' || c == 'W')
					return i - 26;
				else
					state = 0;
				break;
		}
	}

	return -1;
}

static int vantrueOffset(const QByteArray &ba)
{
	int state = 0, cnt;

	for (int i = 16; i < ba.size() - 32; i++) {
		char c = ba.at(i);

		switch (state) {
			case 0:
				if (c == 'A') {
					state = 1;
					cnt = 0;
				}
				break;
			case 1:
				if (cnt == 11 && (c == 'N' || c == 'S')) {
					state = 2;
					cnt = 0;
				} else if (cnt >= 11)
					state = 0;
				else
					cnt++;
				break;
			case 2:
				if (cnt == 15 && (c == 'E' || c == 'W'))
					return i - 40;
				else if (cnt >= 15)
					state = 0;
				else
					cnt++;
				break;
		}
	}

	return -1;
}

static void novatekData(QDataStream &stream, double &lat, double &lon,
  double &speed, quint32 &h, quint32 &m, quint32 &s, quint32 &y, quint32 &M,
  quint32 &d)
{
	float flat, flon, fspeed;
	quint8 fix, u1, EW, NS;

	stream >> h >> m >> s >> y >> M >> d >> fix >> NS >> EW >> u1;
	stream.readRawData((char*)&flat, sizeof(flat));
	stream.readRawData((char*)&flon, sizeof(flon));
	stream.readRawData((char*)&fspeed, sizeof(fspeed));

	lon = lon2dd(flon, EW);
	lat = lat2dd(flat, NS);
	speed = fspeed * 0.51444;
	y += 2000;
}

static void vantrueData(QDataStream &stream, double &lat, double &lon,
  double &speed, quint32 &h, quint32 &m, quint32 &s, quint32 &y, quint32 &M,
  quint32 &d)
{
	quint32 u4;
	quint8 EW, NS;

	stream >> h >> m >> s >> u4;
	stream.readRawData((char*)&lat, sizeof(lat));
	stream >> NS;
	stream.skipRawData(7);
	stream.readRawData((char*)&lon, sizeof(lon));
	stream >> EW;
	stream.skipRawData(7);
	stream.readRawData((char*)&speed, sizeof(speed));
	stream.skipRawData(8);
	stream >> y >> M >> d;

	lon = lon2dd(lon, EW);
	lat = lat2dd(lat, NS);
	speed *= 0.51444;
	y += 2000;
}

bool MP4Parser::novatek(QFile *file, quint64 offset, quint32 size,
  SegmentData &segment)
{
	if (!file->seek(offset)) {
		_errorString = "Invalid Novatek sample offset";
		return false;
	}

	QDataStream stream(file);
	stream.setByteOrder(QDataStream::BigEndian);

	quint32 atomType, atomSize, magic;
	stream >> atomSize >> atomType >> magic;
	if (stream.status() || atomSize != size || atomType != FREE
	  || magic != CGPS) {
		_errorString = "Invalid Novatek data";
		return false;
	}

	QByteArray ba(atomSize - 12, Qt::Initialization::Uninitialized);
	if (stream.readRawData(ba.data(), ba.size()) != ba.size()) {
		_errorString = "Unexpected Novatek EOF";
		return false;
	}

	quint32 h, m, s, y, M, d;
	double lat, lon, speed;
	int gpsOffset;
	QBuffer buf(&ba);
	buf.open(QIODevice::ReadOnly);
	QDataStream ds(&buf);
	ds.setByteOrder(QDataStream::LittleEndian);

	if ((gpsOffset = novatekOffset(ba)) >= 0) {
		ds.skipRawData(gpsOffset);
		novatekData(ds, lat, lon, speed, h, m, s, y, M, d);
	} else if ((gpsOffset = vantrueOffset(ba)) >= 0) {
		ds.skipRawData(gpsOffset);
		vantrueData(ds, lat, lon, speed, h, m, s, y, M, d);
	} else {
		qWarning("%s: Unknown Novatek GPS data format",
		  qUtf8Printable(file->fileName()));
		return true;
	}

	Trackpoint tp(Coordinates(lon, lat));
	tp.setTimestamp(QDateTime(QDate(y, M, d), QTime(h, m, s),
	  QTimeZone::utc()));
	tp.setSpeed(speed);
	if (!tp.coordinates().isValid()) {
		_errorString = "Unknown/obfuscated Novatek GPS data format";
		return false;
	} else
		segment.append(tp);

	return true;
}

bool MP4Parser::ligoJSON(QFile *file, quint64 offset, quint32 size,
  SegmentData &segment)
{
	if (!file->seek(offset)) {
		_errorString = "Invalid LigoJSON offset";
		return false;
	}

	quint32 gpsOffset, sampleSize;
	QDataStream stream(file);
	stream.setByteOrder(QDataStream::LittleEndian);
	stream >> gpsOffset >> sampleSize;
	if (gpsOffset > size || stream.skipRawData(gpsOffset - 8) != gpsOffset - 8) {
		_errorString = "Invalid LigoJSON GPS offset";
		return false;
	}
	quint32 samples = (size - gpsOffset) / sampleSize;
	QByteArray sample(sampleSize, Qt::Initialization::Uninitialized);

	for (quint32 i = 0; i < samples; i++) {
		if (stream.readRawData(sample.data(), sample.size()) != sample.size()) {
			_errorString = "Unexpected LigoJSON sample EOF";
			return false;
		}
		if (!sample.startsWith("LIGOGPSINFO ")) {
			_errorString = "Invalid LigoJSON sample data";
			return false;
		}

		QByteArray json(sample.mid(12, sample.lastIndexOf('}') - 11));
		QJsonParseError err;
		QJsonDocument doc(QJsonDocument::fromJson(json, &err));
		if (err.error) {
			_errorString = err.errorString();
			return false;
		}

		if (doc.object().value("status").toString() != "A")
			continue;

		bool lonOk, latOk;
		double lon = doc.object().value("Longitude").toString().toDouble(&lonOk);
		double lat = doc.object().value("Latitude").toString().toDouble(&latOk);
		Coordinates c(lon, lat);
		if (!(lonOk && latOk && c.isValid())) {
			_errorString = "Invalid/missing LigoJSON coordinates";
			return false;
		}
		if (doc.object().value("EW").toString() == "W")
			c.rlon() = -c.lon();
		if (doc.object().value("NS").toString() == "S")
			c.rlat() = -c.lat();

		bool yOk, mOk, dOk, hOk, minOk, sOk;
		unsigned y = doc.object().value("Year").toString().toUInt(&yOk);
		unsigned m = doc.object().value("Month").toString().toUInt(&mOk);
		unsigned d = doc.object().value("Day").toString().toUInt(&dOk);
		unsigned h = doc.object().value("Hour").toString().toUInt(&hOk);
		unsigned min = doc.object().value("Minute").toString().toUInt(&minOk);
		unsigned s = doc.object().value("Second").toString().toUInt(&sOk);

		bool speedOk;
		double speed = doc.object().value("Speed").toString().toDouble(&speedOk);

		Trackpoint tp(c);
		if (yOk && mOk && dOk && hOk && minOk && sOk)
			tp.setTimestamp(QDateTime(QDate(y, m, d), QTime(h, min, s)));
		if (speedOk)
			tp.setSpeed(speed * 0.514444);
		segment.append(tp);
	}

	return true;
}

bool MP4Parser::pittasoft(QFile *file, quint64 offset, quint32 size,
  SegmentData &segment)
{
	char line[1024];
	NMEAParser parser;
	NMEAParser::CTX ctx;

	if (!file->seek(offset)) {
		_errorString = "Invalid Pittasoft offset";
		return false;
	}

	while (size) {
		qint64 len = file->readLine(line, qMin((quint32)sizeof(line), size));

		if (len < 0) {
			_errorString = "NMEA I/O error";
			return false;
		} else if (len >= (qint64)sizeof(line) - 1) {
			char *eol = (char*)memchr(line, 0, len);
			if (eol) {
				len -= (eol - line);
				size = 0;
			} else {
				_errorString = "NMEA line limit exceeded";
				return false;
			}
		} else
			size -= len;

		char *nmea = line;
		for (qint64 i = 0; i < len; i++) {
			if (*nmea == '$')
				break;
			else
				nmea++;
		}
		len -= (nmea - line);

		if (len >= 12) {
			if (!memcmp(nmea + 3, "RMC,", 4)) {
				if (!parser.readRMC(ctx, nmea + 7, len - 7, segment)) {
					_errorString = parser.errorString();
					return false;
				}
			} else if (!memcmp(nmea + 3, "GGA,", 4)) {
				if (!parser.readGGA(ctx, nmea + 7, len - 7, segment)) {
					_errorString = parser.errorString();
					return false;
				}
			} else if (!memcmp(nmea + 3, "ZDA,", 4)) {
				if (!parser.readZDA(ctx, nmea + 7, len - 7)) {
					_errorString = parser.errorString();
					return false;
				}
			}
		}
	}

	return true;
}

bool MP4Parser::metadata(QFile *file, const Metadata &meta, SegmentData &segment)
{
	if (!meta.format)
		return true;
	if (meta.tables.isEmpty()) {
		_errorString = "Missing stsc table";
		return false;
	}

	int cnt = 0, ti = 0;

	for (int i = 0; i < meta.chunks.size(); i++) {
		if (ti + 1 < meta.tables.size()
		  && i >= (int)(meta.tables.at(ti + 1).first - 1))
			ti++;

		quint32 offset = meta.chunks.at(i);
		const Table &t = meta.tables.at(ti);

		for (quint32 j = 0; j < t.samples; j++) {
			if (meta.sizes.size() <= cnt) {
				_errorString = "Invalid number of stsz entries";
				return false;
			}
			quint32 size = meta.sizes.at(cnt);

			if (meta.id == t.id) {
				switch (meta.format) {
					case GPMDFormat:
						if (!gpmf(file, offset, size, segment))
							return false;
						break;
					case RTMDFormat:
						if (!rtmf(file, offset, size, segment))
							return false;
						break;
					case CAMMFormat:
						if (!camm(file, offset, size, segment))
							return false;
						break;
					case NovatekFormat:
						if (!novatek(file, offset, size, segment))
							return false;
						break;
					case LigoJSONFormat:
						if (!ligoJSON(file, offset, size, segment))
							return false;
						break;
					case PittasoftFormat:
						if (!pittasoft(file, offset, size, segment))
							return false;
						break;
					default:
						break;
				}
			}

			cnt++;
			offset += size;
		}
	}

	return true;
}

bool MP4Parser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons, QVector<Waypoint> &waypoints)
{
	Q_UNUSED(routes);
	Q_UNUSED(polygons);
	Metadata meta;
	SegmentData segment;
	Waypoint wpt;

	if (!mp4(file, meta, wpt)) {
		QString es(_errorString);

		if (gpmf(file, 0, file->size(), segment)) {
			if (segment.size()) {
				TrackData t(segment);
				t.setFile(file->fileName());
				tracks.append(t);
				return true;
			} else
				_errorString = "No GPS data found in GPMF";
		} else
			_errorString = es;
	} else {
		if (!metadata(file, meta, segment))
			return false;

		if (segment.size()) {
			TrackData t(segment);
			t.setFile(file->fileName());
			t.markVideo(true);
			tracks.append(t);
			return true;
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
