#include <QDataStream>
#include <QTextCodec>
#include <QtEndian>
#include <QUrl>
#include "gpiparser.h"


struct RecordHeader {
	quint16 type;
	quint16 flags;
	quint32 size;
	quint32 extra;
};

class TranslatedString {
public:
	TranslatedString() {}
	TranslatedString(const QString &lang, const QString &str)
		: _lang(lang), _str(str) {}

	const QString &str() const {return _str;}
	const QString &lang() const {return _lang;}

private:
	QString _lang;
	QString _str;
};

static inline double toWGS(qint32 v)
{
	return (double)(((double)v / (double)(1U<<31)) * (double)180);
}

static quint16 nextHeaderType(QDataStream &stream)
{
	quint16 type = 0;
	stream.device()->peek((char*)&type, sizeof(type));
	return qFromLittleEndian(type);
}

static quint8 readRecordHeader(QDataStream &stream, RecordHeader &hdr)
{
	stream >> hdr.type >> hdr.flags >> hdr.size;
	if (hdr.flags & 0xA)
		stream >> hdr.extra;
	return (hdr.flags & 0xA) ? 12 : 8;
}

static quint32 skipRecord(QDataStream &stream)
{
	RecordHeader rh;
	quint8 rs = readRecordHeader(stream, rh);
	stream.skipRawData(rh.size);

	return rs + rh.size;
}

static quint32 readFprsRecord(QDataStream &stream)
{
	RecordHeader rh;
	quint16 s1;
	quint8 rs, s2, s3, s4;

	rs = readRecordHeader(stream, rh);
	stream >> s1 >> s2 >> s3 >> s4;

	return rs + 5;
}

static quint16 readString(QDataStream &stream, QTextCodec *codec, QString &str)
{
	quint16 len;
	stream >> len;
	QByteArray ba;
	ba.resize(len);
	stream.readRawData(ba.data(), len);
	str = codec ? codec->toUnicode(ba) : QString::fromLatin1(ba);

	return len + 2;
}

static quint32 readTranslatedObjects(QDataStream &stream, QTextCodec *codec,
  QList<TranslatedString> &objects)
{
	qint32 size = 0, ret;
	char lang[2];

	objects.clear();

	stream >> size;
	ret = size + 4;
	while (size > 0) {
		QString str;
		stream.readRawData(lang, sizeof(lang));
		size -= readString(stream, codec, str) + 2;
		objects.append(TranslatedString(lang, str));
	}

	if (size < 0)
		stream.setStatus(QDataStream::ReadCorruptData);

	return ret;
}

static quint32 readDescription(QDataStream &stream, QTextCodec *codec,
  Waypoint &waypoint)
{
	RecordHeader rh;
	quint8 rs;
	quint32 ds;
	QList<TranslatedString> obj;

	rs = readRecordHeader(stream, rh);
	ds = readTranslatedObjects(stream, codec, obj);
	if (!obj.isEmpty())
		waypoint.setDescription(obj.first().str());

	if (ds != rh.size)
		stream.setStatus(QDataStream::ReadCorruptData);

	return rs + rh.size;
}

static quint32 readNotes(QDataStream &stream, QTextCodec *codec,
  Waypoint &waypoint)
{
	RecordHeader rh;
	quint8 rs, s1;
	quint32 ds = 1;

	rs = readRecordHeader(stream, rh);
	stream >> s1;
	if (s1 & 0x1) {
		QList<TranslatedString> obj;
		ds += readTranslatedObjects(stream, codec, obj);
		if (!obj.isEmpty() && waypoint.description().isNull())
			waypoint.setDescription(obj.first().str());
	}
	if (s1 & 0x2) {
		QString str;
		ds += readString(stream, codec, str);
		if (!str.isEmpty() && waypoint.description().isNull())
			waypoint.setDescription(str);
	}

	if (ds != rh.size)
		stream.setStatus(QDataStream::ReadCorruptData);

	return rs + rh.size;
}

static quint32 readContact(QDataStream &stream, QTextCodec *codec,
  Waypoint &waypoint)
{
	RecordHeader rh;
	quint8 rs;
	quint16 s1;
	quint32 ds = 2;
	QString str;
	QList<TranslatedString> obj;

	rs = readRecordHeader(stream, rh);
	stream >> s1;

	if (s1 & 0x1) // phone
		ds += readString(stream, codec, str);
	if (s1 & 0x2) // phone2
		ds += readString(stream, codec, str);
	if (s1 & 0x4) // fax
		ds += readString(stream, codec, str);
	if (s1 & 0x8) // mail
		ds += readString(stream, codec, str);
	if (s1 & 0x10) { // web
		ds += readString(stream, codec, str);
		QUrl url(str);
		waypoint.addLink(Link(url.scheme().isEmpty()
		  ? "http://" + str : str, str));
	}
	if (s1 & 0x20) // unknown
		ds += readTranslatedObjects(stream, codec, obj);

	if (ds != rh.size)
		stream.setStatus(QDataStream::ReadCorruptData);

	return rs + rh.size;
}

static quint32 readPOI(QDataStream &stream, QTextCodec *codec,
  QVector<Waypoint> &waypoints)
{
	RecordHeader rh;
	quint8 rs;
	quint32 ds;
	qint32 s1, s2;
	quint16 s3;
	QList<TranslatedString> obj;

	rs = readRecordHeader(stream, rh);
	stream >> s1 >> s2 >> s3;
	stream.skipRawData(s3);
	ds = 10 + s3;
	ds += readTranslatedObjects(stream, codec, obj);

	waypoints.append(Waypoint(Coordinates(toWGS(s2), toWGS(s1))));
	if (!obj.isEmpty())
		waypoints.last().setName(obj.first().str());

	while (ds < rh.size) {
		switch(nextHeaderType(stream)) {
			case 10:
				ds += readDescription(stream, codec, waypoints.last());
				break;
			case 12:
				ds += readContact(stream, codec, waypoints.last());
				break;
			case 14:
				ds += readNotes(stream, codec, waypoints.last());
				break;
			default:
				ds += skipRecord(stream);
		}
	}

	if (ds != rh.size)
		stream.setStatus(QDataStream::ReadCorruptData);

	return rs + rh.size;
}

static quint32 readSpatialIndex(QDataStream &stream, QTextCodec *codec,
  QVector<Waypoint> &waypoints)
{
	RecordHeader rh;
	quint32 ds, s5;
	qint32 top, right, bottom, left;
	quint16 s6;
	quint8 rs;

	rs = readRecordHeader(stream, rh);
	stream >> top >> right >> bottom >> left >> s5 >> s6;
	stream.skipRawData(s6);
	ds = 22 + s6;
	if (rh.flags & 0x8) {
		while (ds < rh.size) {
			switch(nextHeaderType(stream)) {
				case 2:
					ds += readPOI(stream, codec, waypoints);
					break;
				case 8:
					ds += readSpatialIndex(stream, codec, waypoints);
					break;
				default:
					ds += skipRecord(stream);
			}
		}
	}

	if (ds != rh.size)
		stream.setStatus(QDataStream::ReadCorruptData);

	return rs + rh.size;
}

static quint32 readFileDataRecord(QDataStream &stream, QTextCodec *codec)
{
	RecordHeader rh;
	quint32 ds, s1;
	quint16 s2, s3;
	quint8 rs;
	QList<TranslatedString> obj;

	rs = readRecordHeader(stream, rh);
	stream >> s1 >> s2 >> s3;
	ds = 8;
	ds += readTranslatedObjects(stream, codec, obj);
	ds += readTranslatedObjects(stream, codec, obj);

	if (s1 & 0x10) {
		quint8 ss1, ss2;
		quint16 ss3;
		stream >> ss1 >> ss2 >> ss3;
		ds += 4;
	}
	if (s1 & 0x100) {
		quint32 ss1;
		stream >> ss1;
		if (ss1)
			stream.skipRawData(ss1);
		ds += ss1 + 4;
	}
	if (s1 & 0x400) {
		QString str;
		ds += readString(stream, codec, str);
	}
	if (s1 & 0x400000) {
		quint16 ss1;
		stream >> ss1;
		if (ss1)
			stream.skipRawData(ss1);
		ds += ss1 + 2;
	}

	if (ds != rh.size)
		stream.setStatus(QDataStream::ReadCorruptData);

	return rs + rh.size;
}

bool GPIParser::readFileHeader(QDataStream &stream)
{
	RecordHeader rh;
	quint32 ds, s7;
	quint16 s10;
	quint8 s5, s6, s8, s9;
	char magic[6];

	readRecordHeader(stream, rh);
	stream.readRawData(magic, sizeof(magic));
	if (memcmp(magic, "GRMREC", sizeof(magic))) {
		_errorString = "Not a GPI file";
		return false;
	}
	stream >> s5 >> s6 >> s7 >> s8 >> s9 >> s10;
	stream.skipRawData(s10);
	ds = sizeof(magic) + 10 + s10;
	if (rh.flags & 8)
		ds += readFprsRecord(stream);

	if (s8 & 0x4) {
		_errorString = "Encrypted GPI files not supported";
		return false;
	}

	if (stream.status() != QDataStream::Ok || ds != rh.size) {
		_errorString = "Invalid file header";
		return false;
	} else
		return true;
}

bool GPIParser::readGPIHeader(QDataStream &stream, QTextCodec *codec)
{
	RecordHeader rh;
	char m1[6], m2[2];
	quint16 codepage = 0;
	quint8 s2, s3;
	quint32 ds;

	readRecordHeader(stream, rh);
	stream.readRawData(m1, sizeof(m1));
	stream.readRawData(m2, sizeof(m2));
	stream >> codepage >> s2 >> s3;
	ds = sizeof(m1) + sizeof(m2) + 4;

	if (codepage == 65001)
		codec = QTextCodec::codecForName("UTF-8");
	else if (codepage == 0)
		codec = 0;
	else
		codec = QTextCodec::codecForName(QString("CP%1").arg(codepage)
		  .toLatin1());

	if (s2 & 0x10)
		ds += readFileDataRecord(stream, codec);

	if (stream.status() != QDataStream::Ok || ds != rh.size) {
		_errorString = "Invalid GPI header";
		return false;
	} else
		return true;
}

void GPIParser::readPOIDatabase(QDataStream &stream, QTextCodec *codec,
  QVector<Waypoint> &waypoints)
{
	RecordHeader rh;
	QList<TranslatedString> obj;
	quint32 ds;

	readRecordHeader(stream, rh);
	ds = readTranslatedObjects(stream, codec, obj);
	ds += readSpatialIndex(stream, codec, waypoints);
	if (rh.flags & 0x8) {
		while (ds < rh.size) {
			switch(nextHeaderType(stream)) {
				case 5: // symbol
				case 7: // category
				default:
					ds += skipRecord(stream);
			}
		}
	}

	if (ds != rh.size)
		stream.setStatus(QDataStream::ReadCorruptData);
}

bool GPIParser::readEntry(QDataStream &stream, QTextCodec *codec,
  QVector<Waypoint> &waypoints)
{
	switch (nextHeaderType(stream)) {
		case 0x09:   // POI database
			readPOIDatabase(stream, codec, waypoints);
			break;
		case 0xffff: // EOF
			skipRecord(stream);
			return false;
		case 0x16:   // route
		case 0x15:   // info header
		default:
			skipRecord(stream);
	}

	return true;
}

bool GPIParser::readData(QDataStream &stream, QTextCodec *codec,
  QVector<Waypoint> &waypoints)
{
	while (stream.status() == QDataStream::Ok)
		if (!readEntry(stream, codec, waypoints))
			return stream.atEnd();

	return false;
}

bool GPIParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons, QVector<Waypoint> &waypoints)
{
	Q_UNUSED(tracks);
	Q_UNUSED(routes);
	Q_UNUSED(polygons);
	QDataStream stream(file);
	QTextCodec *codec = 0;

	stream.setByteOrder(QDataStream::LittleEndian);

	if (!readFileHeader(stream) || !readGPIHeader(stream, codec))
		return false;

	if (!readData(stream, codec, waypoints)) {
		_errorString = "Invalid/corrupted GPI data";
		return false;
	} else
		return true;
}
