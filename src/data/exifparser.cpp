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

static double altitude(TIFFFile &file, quint32 offset)
{
	if (!file.seek(offset))
		return NAN;

	quint32 num, den;
	if (!file.readValue(num))
		return false;
	if (!file.readValue(den))
		return false;

	return num/den;
}

static double coordinate(TIFFFile &file, quint32 offset)
{
	if (!file.seek(offset))
		return NAN;

	double dms[3];

	for (int i = 0; i < 3; i++) {
		quint32 num, den;
		if (!file.readValue(num))
			return false;
		if (!file.readValue(den))
			return false;

		dms[i] = num/den;
	}

	return dms[0] + dms[1]/60 + dms[2]/3600;
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

bool EXIFParser::parseTIFF(QDataStream &stream, QVector<Waypoint> &waypoints)
{
	quint16 size;
	char magic[6];

	stream >> size;
	if (stream.readRawData(magic, sizeof(magic)) != sizeof(magic) ||
	  memcmp(magic, "Exif\0\0", sizeof(magic))) {
		_errorString = "No EXIF data found";
		return false;
	}

	qint64 offset = stream.device()->pos();

	TIFFFile tiff(stream.device());
	if (!tiff.isValid()) {
		_errorString = "Invalid EXIF data";
		return false;
	}

	QSet<quint16> exifTags;
	exifTags.insert(GPSIFDTag);
	QMap<quint16, IFDEntry> exifEntries;
	for (quint32 ifd = tiff.ifd(); ifd; ) {
		if (!readIFD(tiff, offset + ifd, exifTags, exifEntries)
		  || !tiff.readValue(ifd)) {
			_errorString = "Invalid EXIF IFD";
			return false;
		}
	}
	if (!exifEntries.contains(GPSIFDTag)) {
		_errorString = "GPS IFD not found";
		return false;
	}

	QSet<quint16> gpsTags;
	gpsTags.insert(GPSLatitude);
	gpsTags.insert(GPSLongitude);
	gpsTags.insert(GPSLatitudeRef);
	gpsTags.insert(GPSLongitudeRef);
	gpsTags.insert(GPSAltitude);
	gpsTags.insert(GPSAltitudeRef);
	QMap<quint16, IFDEntry> gpsEntries;
	for (quint32 ifd = exifEntries.value(GPSIFDTag).offset; ifd; ) {
		if (!readIFD(tiff, offset + ifd, gpsTags, gpsEntries)
		  || !tiff.readValue(ifd)) {
			_errorString = "Invalid GPS IFD";
			return false;
		}
	}

	IFDEntry lat(gpsEntries.value(GPSLatitude));
	IFDEntry lon(gpsEntries.value(GPSLongitude));
	IFDEntry latRef(gpsEntries.value(GPSLatitudeRef));
	IFDEntry lonRef(gpsEntries.value(GPSLongitudeRef));
	if (!(lat.type == TIFF_RATIONAL && lat.count == 3
	  && lon.type == TIFF_RATIONAL && lon.count == 3
	  && latRef.type == TIFF_ASCII && latRef.count == 2
	  && lonRef.type == TIFF_ASCII && lonRef.count == 2)) {
		_errorString = "Invalid/missing GPS IFD Lat/Lon entry";
		return false;
	}

	Coordinates c(coordinate(tiff, offset + lon.offset),
	  coordinate(tiff, offset + lat.offset));
	if (lonRef.offset == 'W')
		c.rlon() = -c.lon();
	if (latRef.offset == 'S')
		c.rlat() = -c.lat();
	if (!c.isValid()) {
		_errorString = "Invalid coordinates";
		return false;
	}

	Waypoint wp(c);
	QFile *file = static_cast<QFile*>(stream.device());
	wp.setName(QFileInfo(file->fileName()).fileName());
	IFDEntry alt(gpsEntries.value(GPSAltitude));
	IFDEntry altRef(gpsEntries.value(GPSAltitudeRef));
	if (alt.type == TIFF_RATIONAL && alt.count == 1 && altRef.type == TIFF_BYTE
	  && altRef.count == 1)
		wp.setElevation(altRef.offset ? -altitude(tiff, alt.offset)
		  : altitude(tiff, alt.offset));

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
		if (marker == 0xFFE1)
			return parseTIFF(stream, waypoints);
	}

	_errorString = "No EXIF data found";
	return false;
}
