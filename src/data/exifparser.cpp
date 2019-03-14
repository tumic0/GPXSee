#include <QDataStream>
#include <QFileInfo>
#include "common/tifffile.h"
#include "exifparser.h"


#define GPSIFDTag       34853
#define GPSLatitudeRef  1
#define GPSLatitude     2
#define GPSLongitudeRef 3
#define GPSLongitude    4
#define GPSAltitudeRef  5
#define GPSAltitude     6
#define GPSTimeStamp    7
#define GPSDateStamp    29

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

QDate EXIFParser::date(TIFFFile &file, const IFDEntry &ds) const
{
	if (!(ds.type == TIFF_ASCII && ds.count == 11))
		return QDate();

	if (!file.seek(ds.offset))
		return QDate();

	QByteArray text(file.read(11));
	if (text.size() < 11)
		return QDate();

	return QDate::fromString(text, "yyyy:MM:dd");
}

double EXIFParser::altitude(TIFFFile &file, const IFDEntry &alt,
  const IFDEntry &altRef) const
{
	if (!(alt.type == TIFF_RATIONAL && alt.count == 1))
		return NAN;

	if (!file.seek(alt.offset))
		return NAN;

	quint32 num, den;
	if (!file.readValue(num))
		return NAN;
	if (!file.readValue(den))
		return NAN;

	return (altRef.type == TIFF_BYTE && altRef.count == 1 && altRef.offset)
	  ? -num/(double)den : num/(double)den;
}

double EXIFParser::coordinate(TIFFFile &file, const IFDEntry &ll) const
{
	if (!(ll.type == TIFF_RATIONAL && ll.count == 3))
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

	if (lonRef.offset == 'W')
		c.rlon() = -c.lon();
	if (latRef.offset == 'S')
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
	if (!file.readValue(entry.count))
		return false;
	if (!file.readValue(entry.offset))
		return false;

	if (tags.contains(tag))
		entries.insert(tag, entry);

	return true;
}

bool EXIFParser::readIFD(TIFFFile &file, quint32 offset,
  const QSet<quint16> &tags, QMap<quint16, IFDEntry> &entries) const
{
	quint16 count;

	if (!file.seek(offset))
		return false;
	if (!file.readValue(count))
		return false;

	for (quint16 i = 0; i < count; i++)
		if (!readEntry(file, tags, entries))
			return false;

	return true;
}

bool EXIFParser::parseTIFF(QFile *file, QVector<Waypoint> &waypoints)
{
	TIFFFile tiff(file);
	if (!tiff.isValid()) {
		_errorString = "Invalid EXIF data";
		return false;
	}

	QSet<quint16> exifTags;
	exifTags << GPSIFDTag;
	QMap<quint16, IFDEntry> exif;
	for (quint32 ifd = tiff.ifd(); ifd; ) {
		if (!readIFD(tiff, ifd, exifTags, exif) || !tiff.readValue(ifd)) {
			_errorString = "Invalid EXIF IFD";
			return false;
		}
	}
	if (!exif.contains(GPSIFDTag)) {
		_errorString = "GPS IFD not found";
		return false;
	}

	QSet<quint16> gpsTags;
	gpsTags << GPSLatitude << GPSLongitude << GPSLatitudeRef << GPSLongitudeRef
	  << GPSAltitude << GPSAltitudeRef << GPSDateStamp << GPSTimeStamp;
	QMap<quint16, IFDEntry> gps;
	for (quint32 ifd = exif.value(GPSIFDTag).offset; ifd; ) {
		if (!readIFD(tiff, ifd, gpsTags, gps) || !tiff.readValue(ifd)) {
			_errorString = "Invalid GPS IFD";
			return false;
		}
	}

	Coordinates c(coordinates(tiff, gps.value(GPSLongitude),
	  gps.value(GPSLongitudeRef), gps.value(GPSLatitude),
	  gps.value(GPSLatitudeRef)));
	if (!c.isValid()) {
		_errorString = "Invalid/missing GPS coordinates";
		return false;
	}

	Waypoint wp(c);
	wp.setName(QFileInfo(file->fileName()).baseName());
	wp.setImage(file->fileName());
	wp.setElevation(altitude(tiff, gps.value(GPSAltitude),
	  gps.value(GPSAltitudeRef)));
	wp.setTimestamp(QDateTime(date(tiff, gps.value(GPSDateStamp)),
	  time(tiff, gps.value(GPSTimeStamp)), Qt::UTC));

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

	QDataStream stream(file);
	stream.setByteOrder(QDataStream::BigEndian);

	quint16 marker;
	while (!stream.atEnd()) {
		stream >> marker;
		if (marker == 0xFFE1) {
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
