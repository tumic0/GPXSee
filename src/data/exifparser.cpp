#include <QDataStream>
#include <QTimeZone>
#include "common/tifffile.h"
#include "common/util.h"
#include "exifparser.h"


#define SOI_MARKER       0xFFD8
#define APP1_MARKER      0xFFE1

#define GPSIFDTag        34853
#define ImageDescription 270

#define GPSLatitudeRef   1
#define GPSLatitude      2
#define GPSLongitudeRef  3
#define GPSLongitude     4
#define GPSAltitudeRef   5
#define GPSAltitude      6
#define GPSTimeStamp     7
#define GPSDateStamp     29


QString EXIFParser::text(TIFFFile &file, const IFDEntry &e) const
{
	if (e.type != TIFF_ASCII || !e.count)
		return QString();

	int threshold = file.isBigTIFF() ? 8 : 4;
	QByteArray ba(e.count, Qt::Initialization::Uninitialized);

	if (e.count <= threshold) {
		if (file.isBigEndian()) {
			for (int i = 0; i < e.count; i++)
				*(ba.data() + i) = *((char*)&e.offset + threshold - 1 - i);
			return ba;
		} else
			memcpy(ba.data(), (char*)&e.offset, ba.size());
	} else {
		if (!file.seek(e.offset))
			return QString();

		if (file.read(ba.data(), ba.size()) < ba.size())
			return QString();
	}

	while (ba.size() && ba.back() == '\0')
		ba.resize(ba.size() - 1);

	return ba;
}

QTime EXIFParser::time(TIFFFile &file, const IFDEntry &ts) const
{
	if (!(ts.type == TIFF_RATIONAL && ts.count == 3))
		return QTime();

	if (!file.seek(ts.offset))
		return QTime();

	double hms[3];
	for (int i = 0; i < 3; i++) {
		quint32 num, den;
		if (!file.readValue(num))
			return QTime();
		if (!file.readValue(den))
			return QTime();

		hms[i] = num/(double)den;
	}

	return QTime((int)hms[0], (int)hms[1], (int)hms[2]);
}

double EXIFParser::altitude(TIFFFile &file, const IFDEntry &alt,
  const IFDEntry &altRef) const
{
	quint32 num, den;

	if (!(alt.type == TIFF_RATIONAL && alt.count == 1))
		return NAN;

	if (file.isBigTIFF()) {
		num = ((quint64)alt.offset) >> 32;
		den = ((quint64)alt.offset) & 0xFFFFFFFF;
	} else {
		if (!file.seek(alt.offset))
			return NAN;

		if (!file.readValue(num))
			return NAN;
		if (!file.readValue(den))
			return NAN;
	}

	return (altRef.type == TIFF_BYTE && altRef.count == 1 && altRef.offset)
	  ? -(num/(double)den) : num/(double)den;
}

double EXIFParser::coordinate(TIFFFile &file, const IFDEntry &ll) const
{
	// Some broken image creators like NOKIA phones use a wrong (SRATIONAL)
	// data type
	if (!((ll.type == TIFF_RATIONAL || ll.type == TIFF_SRATIONAL)
	  && ll.count == 3))
		return NAN;

	if (!file.seek(ll.offset))
		return NAN;

	double dms[3];
	for (int i = 0; i < 3; i++) {
		quint32 num, den;
		if (!file.readValue(num))
			return NAN;
		if (!file.readValue(den))
			return NAN;

		dms[i] = num/(double)den;
	}

	return dms[0] + dms[1]/60 + dms[2]/3600;
}

Coordinates EXIFParser::coordinates(TIFFFile &file, const IFDEntry &lon,
  const IFDEntry &lonRef, const IFDEntry &lat, const IFDEntry &latRef) const
{
	if (!(latRef.type == TIFF_ASCII && latRef.count == 2
	  && lonRef.type == TIFF_ASCII && lonRef.count == 2))
		return Coordinates();

	Coordinates c(coordinate(file, lon), coordinate(file, lat));
	if (!c.isValid())
		return Coordinates();

	int threshold = file.isBigTIFF() ? 8 : 4;
	char ew = file.isBigEndian()
	  ? lonRef.offset >> ((threshold - 1) * 8)
	  : lonRef.offset;
	char ns = file.isBigEndian()
	  ? latRef.offset >> ((threshold - 1) * 8)
	  : latRef.offset;

	if (ew == 'W')
		c.rlon() = -c.lon();
	if (ns == 'S')
		c.rlat() = -c.lat();

	return c;
}

bool EXIFParser::readEntry(TIFFFile &file, const QSet<quint16> &tags,
  QMap<quint16, IFDEntry> &entries) const
{
	IFDEntry entry;
	quint16 tag;

	if (!file.readValue(tag))
		return false;
	if (!file.readValue(entry.type))
		return false;
	if (file.isBigTIFF()) {
		if (!file.readValue(entry.count))
			return false;
		if (!file.readValue(entry.offset))
			return false;
	} else {
		quint32 count, offset;
		if (!file.readValue(count))
			return false;
		entry.count = count;
		if (!file.readValue(offset))
			return false;
		entry.offset = offset;
	}

	if (tags.contains(tag))
		entries.insert(tag, entry);

	return true;
}

bool EXIFParser::readIFD(TIFFFile &file, qint64 offset,
  const QSet<quint16> &tags, QMap<quint16, IFDEntry> &entries) const
{
	qint64 count;

	if (!file.seek(offset))
		return false;
	if (file.isBigTIFF()) {
		if (!file.readValue(count))
			return false;
	} else {
		quint16 count16;
		if (!file.readValue(count16))
			return false;
		count = count16;
	}

	for (qint64 i = 0; i < count; i++)
		if (!readEntry(file, tags, entries))
			return false;

	return true;
}

static bool nextIFD(TIFFFile &file, qint64 &offset)
{
	if (file.isBigTIFF()) {
		if (!file.readValue(offset))
			return false;
	} else {
		quint32 offset32;
		if (!file.readValue(offset32))
			return false;
		offset = offset32;
	}

	return true;
}

bool EXIFParser::parseTIFF(QFile *file, QVector<Waypoint> &waypoints)
{
	TIFFFile tiff(file);
	if (!tiff.isValid()) {
		_errorString = "Invalid EXIF data";
		return false;
	}

	QSet<quint16> IFD0Tags;
	IFD0Tags << GPSIFDTag << ImageDescription;
	QMap<quint16, IFDEntry> IFD0;
	for (qint64 ifd = tiff.ifd(); ifd; ) {
		if (!readIFD(tiff, ifd, IFD0Tags, IFD0)) {
			_errorString = "Invalid IFD0";
			return false;
		}
		if (!nextIFD(tiff, ifd)) {
			_errorString = "Error reading next IFD0";
			return false;
		}
	}
	if (!IFD0.contains(GPSIFDTag)) {
		_errorString = "GPS IFD not found";
		return false;
	}

	QSet<quint16> GPSIFDTags;
	GPSIFDTags << GPSLatitude << GPSLongitude << GPSLatitudeRef
	  << GPSLongitudeRef << GPSAltitude << GPSAltitudeRef << GPSDateStamp
	  << GPSTimeStamp;
	QMap<quint16, IFDEntry> GPSIFD;
	for (qint64 ifd = IFD0.value(GPSIFDTag).offset; ifd; ) {
		if (!readIFD(tiff, ifd, GPSIFDTags, GPSIFD)) {
			_errorString = "Invalid GPS IFD";
			return false;
		}
		if (!nextIFD(tiff, ifd)) {
			_errorString = "Error reading next GPS IFD";
			return false;
		}
	}

	Coordinates c(coordinates(tiff, GPSIFD.value(GPSLongitude),
	  GPSIFD.value(GPSLongitudeRef), GPSIFD.value(GPSLatitude),
	  GPSIFD.value(GPSLatitudeRef)));
	if (!c.isValid()) {
		_errorString = "Invalid/missing GPS coordinates";
		return false;
	}

	Waypoint wp(c);
	wp.setName(Util::file2name(file->fileName()));
	wp.addImage(file->fileName());
	if (GPSIFD.contains(GPSAltitude))
		wp.setElevation(altitude(tiff, GPSIFD.value(GPSAltitude),
		  GPSIFD.value(GPSAltitudeRef)));
	if (GPSIFD.contains(GPSDateStamp) && GPSIFD.contains(GPSTimeStamp))
		wp.setTimestamp(QDateTime(QDate::fromString(text(tiff,
		  GPSIFD.value(GPSDateStamp)), "yyyy:MM:dd"), time(tiff,
		  GPSIFD.value(GPSTimeStamp)), QTimeZone::utc()));
	if (IFD0.contains(ImageDescription))
		wp.setDescription(text(tiff, IFD0.value(ImageDescription)).trimmed());

	waypoints.append(wp);

	return true;
}

bool EXIFParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons,
  QVector<Waypoint> &waypoints)
{
	Q_UNUSED(tracks);
	Q_UNUSED(routes);
	Q_UNUSED(polygons);
	quint16 marker;

	QDataStream stream(file);
	stream.setByteOrder(QDataStream::BigEndian);
	stream >> marker;
	if (marker != SOI_MARKER) {
		_errorString = "Not a JPEG file";
		return false;
	}

	while (!stream.atEnd()) {
		stream >> marker;
		if (marker == APP1_MARKER) {
			quint16 size;
			char magic[6];
			stream >> size;
			if (stream.readRawData(magic, sizeof(magic)) == sizeof(magic) &&
			  !memcmp(magic, "Exif\0\0", sizeof(magic)))
				return parseTIFF(file, waypoints);
			else
				break;
		}
	}

	_errorString = "No EXIF data found";
	return false;
}
