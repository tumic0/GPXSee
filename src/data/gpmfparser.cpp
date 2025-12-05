#include <QDataStream>
#include <QDateTime>
#include <QTimeZone>
#include "gpmfparser.h"

#define alignup(n, k) (((n) + (k) - 1) / (k) * (k))

static constexpr quint32 TAG(const char name[4])
{
	return static_cast<quint32>(name[0] << 24)
	  + (static_cast<quint32>(name[1]) << 16)
	  + (static_cast<quint32>(name[2]) << 8)
	  + (static_cast<quint32>(name[3]));
}

constexpr quint32 FTYP = TAG("ftyp");
constexpr quint32 MOOV = TAG("moov");
constexpr quint32 MP41 = TAG("mp41");
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

constexpr quint32 GPS5 = TAG("GPS5");
constexpr quint32 GPS9 = TAG("GPS9");
constexpr quint32 GPSU = TAG("GPSU");
constexpr quint32 SCAL = TAG("SCAL");

static bool hdr(QDataStream &stream, quint32 &type, quint64 &size)
{
	quint32 size32;

	stream >> size32 >> type;
	if (stream.status())
		return false;
	if (size32 == 1) {
		stream >> size;
		if (stream.status())
			return false;
	} else
		size = size32;

	return true;
}

static bool ftyp(QDataStream &stream)
{
	quint32 type, brand;
	quint64 size;

	if (!hdr(stream, type, size))
		return false;
	stream >> brand;
	if (stream.status())
		return false;

	if (type != FTYP || brand != MP41)
		return false;

	return (stream.skipRawData(size - 12) == (qint64)size - 12);
}

static bool entry(QDataStream &stream, quint32 size, SegmentData &segment,
  bool &gps9, bool &gps5)
{
	quint32 key;
	quint8 ss, type;
	quint16 repeat;
	QDateTime base;
	QByteArray date(16, Qt::Initialization::Uninitialized);
	qint32 scale[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
	static QDateTime dt2000 = QDateTime(QDate(2000, 1, 1), QTime(0, 0));

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

bool GPMFParser::stsd(QDataStream &stream, quint64 atomSize, bool &gpmd)
{
	if (atomSize && atomSize < 8) {
		_errorString = "Invalid STSD atom size";
		return false;
	}

	quint32 vf, num;
	stream >> vf >> num;
	if (stream.status()) {
		_errorString = "Error reading STSD data";
		return false;
	}

	for (quint32 i = 0; i < num; i++) {
		quint32 ds, format;
		stream >> ds >> format;
		stream.skipRawData(ds - 8);
		if (stream.status()) {
			_errorString = "Error reading STSD table entry";
			return false;
		}
		if (format == GPMD)
			gpmd = true;
	}

	return true;
}

bool GPMFParser::stsz(QDataStream &stream, quint64 atomSize,
  QVector<quint32> &sizes)
{
	if (atomSize && atomSize < 12) {
		_errorString = "Invalid STSZ atom size";
		return false;
	}

	quint32 vf, ss, num;
	stream >> vf >> ss >> num;
	if (stream.status()) {
		_errorString = "Error reading STSZ data";
		return false;
	}

	if (ss)
		sizes = QVector<quint32>(num, ss);
	else {
		sizes.resize(num);
		for (quint32 i = 0; i < num; i++)
			stream >> sizes[i];
		if (stream.status()) {
			_errorString = "Error reading STSZ table";
			return false;
		}
	}

	return true;
}

bool GPMFParser::stco(QDataStream &stream, quint64 atomSize,
  QVector<quint64> &chunks)
{
	if (atomSize && atomSize < 8) {
		_errorString = "Invalid STCO atom size";
		return false;
	}

	quint32 vf, num, chunk;
	stream >> vf >> num;
	if (stream.status()) {
		_errorString = "Error reading STCO data";
		return false;
	}

	chunks.resize(num);
	for (quint32 i = 0; i < num; i++) {
		stream >> chunk;
		chunks[i] = chunk;
	}
	if (stream.status()) {
		_errorString = "Error reading STCO table";
		return false;
	}

	return true;
}

bool GPMFParser::co64(QDataStream &stream, quint64 atomSize,
  QVector<quint64> &chunks)
{
	if (atomSize && atomSize < 8) {
		_errorString = "Invalid CO64 atom size";
		return false;
	}

	quint32 vf, num;
	stream >> vf >> num;
	if (stream.status()) {
		_errorString = "Error reading CO64 data";
		return false;
	}

	chunks.resize(num);
	for (quint32 i = 0; i < num; i++)
		stream >> chunks[i];
	if (stream.status()) {
		_errorString = "Error reading CO64 table";
		return false;
	}

	return true;
}

bool GPMFParser::stbl(QDataStream &stream, quint64 atomSize,
  QVector<quint32> &sizes, QVector<quint64> &chunks)
{
	quint32 type;
	quint64 size;
	bool gpmd = false;

	do {
		if (!hdr(stream, type, size)) {
			_errorString = "Error reading STBL atom header";
			return false;
		}
		if (atomSize) {
			if (size > atomSize) {
				_errorString = "STBL size overflow";
				return false;
			} else
				atomSize -= size;
		}

		if (type == STSD) {
			if (!stsd(stream, size ? size - 8 : 0, gpmd))
				return false;
		} else if (type == STSZ && gpmd) {
			if (!stsz(stream, size ? size - 8 : 0, sizes))
				return false;
		} else if (type == STCO && gpmd) {
			if (!stco(stream, size ? size - 8 : 0, chunks))
				return false;
		} else if (type == CO64 && gpmd) {
			if (!co64(stream, size ? size - 8 : 0, chunks))
				return false;
		} else {
			if (size) {
				if (stream.skipRawData(size - 8) != (qint64)size - 8) {
					_errorString = "Unexpected EOF in STBL";
					return false;
				}
			} else
				break;
		}
		if (stream.atEnd())
			break;
	} while (atomSize);

	if (atomSize) {
		_errorString = "STBL size underflow";
		return false;
	}
	if (chunks.size() != sizes.size()) {
		_errorString = "STSZ/STCO size mismatch";
		return false;
	}

	return true;
}

bool GPMFParser::hdlr(QDataStream &stream, quint64 atomSize, bool &mhlr)
{
	// HDLR atom size can not be zero
	if (atomSize < 24) {
		_errorString = "Invalid HDLR atom size";
		return false;
	}

	quint32 vf, type, subtype, manufacturer, flags, mask;
	stream >> vf >> type >> subtype >> manufacturer >> flags >> mask;
	if (stream.status()
	  || stream.skipRawData(atomSize - 24) != (qint64)atomSize - 24) {
		_errorString = "Error reading HDLR data";
		return false;
	}

	mhlr = (type == MHLR && subtype == META);

	return true;
}

bool GPMFParser::minf(QDataStream &stream, quint64 atomSize,
  QVector<quint32> &sizes, QVector<quint64> &chunks)
{
	quint32 type;
	quint64 size;

	do {
		if (!hdr(stream, type, size)) {
			_errorString = "Error reading MINF atom header";
			return false;
		}
		if (atomSize) {
			if (size > atomSize) {
				_errorString = "MINF size overflow";
				return false;
			} else
				atomSize -= size;
		}

		if (type == STBL) {
			if (!stbl(stream, size ? size - 8 : 0, sizes, chunks))
				return false;
		} else {
			if (size) {
				if (stream.skipRawData(size - 8) != (qint64)size - 8) {
					_errorString = "Unexpected EOF in MINF";
					return false;
				}
			} else
				break;
		}
		if (stream.atEnd())
			break;
	} while (atomSize);

	if (atomSize) {
		_errorString = "MINF size underflow";
		return false;
	} else
		return true;
}

bool GPMFParser::mdia(QDataStream &stream, quint64 atomSize,
  QVector<quint32> &sizes, QVector<quint64> &chunks)
{
	quint32 type;
	quint64 size;
	bool mhlr = false;

	do {
		if (!hdr(stream, type, size)) {
			_errorString = "Error reading MDIA atom header";
			return false;
		}
		if (atomSize) {
			if (size > atomSize) {
				_errorString = "MDIA size overflow";
				return false;
			} else
				atomSize -= size;
		}

		if (type == HDLR) {
			if (!hdlr(stream, size ? size - 8 : 0, mhlr))
				return false;
		} else if (type == MINF && mhlr) {
			if (!minf(stream, size ? size - 8 : 0, sizes, chunks))
				return false;
		} else {
			if (size) {
				if (stream.skipRawData(size - 8) != (qint64)size - 8) {
					_errorString = "Unexpected EOF in MDIA";
					return false;
				}
			} else
				break;
		}
		if (stream.atEnd())
			break;
	} while (atomSize);

	if (atomSize) {
		_errorString = "MDIA size underflow";
		return false;
	} else
		return true;
}

bool GPMFParser::trak(QDataStream &stream, quint64 atomSize,
  QVector<quint32> &sizes, QVector<quint64> &chunks)
{
	quint32 type;
	quint64 size;

	do {
		if (!hdr(stream, type, size)) {
			_errorString = "Error reading TRAK atom header";
			return false;
		}
		if (atomSize) {
			if (size > atomSize) {
				_errorString = "TRAK size overflow";
				return false;
			} else
				atomSize -= size;
		}

		if (type == MDIA) {
			if (!mdia(stream, size ? size - 8 : 0, sizes, chunks))
				return false;
		} else {
			if (size) {
				if (stream.skipRawData(size - 8) != (qint64)size - 8) {
					_errorString = "Unexpected EOF in TRAK";
					return false;
				}
			} else
				break;
		}
		if (stream.atEnd())
			break;
	} while (atomSize);

	if (atomSize) {
		_errorString = "TRAK size underflow";
		return false;
	} else
		return true;
}

bool GPMFParser::moov(QDataStream &stream, quint64 atomSize,
  QVector<quint32> &sizes, QVector<quint64> &chunks)
{
	quint32 type;
	quint64 size;

	do {
		if (!hdr(stream, type, size)) {
			_errorString = "Error reading MOOV atom header";
			return false;
		}
		if (atomSize) {
			if (size > atomSize) {
				_errorString = "MOOV size overflow";
				return false;
			} else
				atomSize -= size;
		}

		if (type == TRAK) {
			if (!trak(stream, size ? size - 8 : 0, sizes, chunks))
				return false;
		} else {
			if (size) {
				if (stream.skipRawData(size - 8) != (qint64)size - 8) {
					_errorString = "Unexpected EOF in MOOV";
					return false;
				}
			} else
				break;
		}
		if (stream.atEnd())
			break;
	} while (atomSize);

	if (atomSize) {
		_errorString = "MOOV size underflow";
		return false;
	} else
		return true;
}

bool GPMFParser::mp4(QFile *file, QVector<quint32> &sizes,
  QVector<quint64> &chunks)
{
	QDataStream stream(file);
	quint32 type;
	quint64 size;

	stream.setByteOrder(QDataStream::BigEndian);

	if (!ftyp(stream)) {
		_errorString = "Not a MP4 file";
		return false;
	}

	do {
		if (!hdr(stream, type, size)) {
			_errorString = "Error reading root atom header";
			return false;
		}

		if (type == MOOV) {
			if (!moov(stream, size ? size - 8 : 0, sizes, chunks))
				return false;
		} else {
			if (size) {
				if (stream.skipRawData(size - 8) != (qint64)size - 8) {
					_errorString = "Unexpected EOF in root";
					return false;
				}
			} else
				break;
		}
	} while (!stream.atEnd());

	if (chunks.isEmpty()) {
		_errorString = "No GPMF data found in MP4";
		return false;
	}

	return true;
}

bool GPMFParser::gpmf(QFile *file, quint64 offset, quint32 size,
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
		_errorString = "Not a GPMF file";
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

bool GPMFParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons, QVector<Waypoint> &waypoints)
{
	Q_UNUSED(routes);
	Q_UNUSED(polygons);
	Q_UNUSED(waypoints);
	QVector<quint32> sizes;
	QVector<quint64> chunks;
	SegmentData segment;

	if (!mp4(file, sizes, chunks)) {
		if (!gpmf(file, 0, file->size(), segment))
			return false;
	} else {
		for (int i = 0; i < chunks.size(); i++)
			if (!gpmf(file, chunks.at(i), sizes.at(i), segment))
				return false;
	}

	if (segment.isEmpty()) {
		_errorString = "No GPS data found in GPMF";
		return false;
	} else
		tracks.append(segment);

	return true;
}
