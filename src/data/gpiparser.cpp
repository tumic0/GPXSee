#include <cstring>
#include <QDataStream>
#include <QtEndian>
#include <QUrl>
#include <QIODevice>
#include <QApplication>
#include <QBuffer>
#include <QImageReader>
#include <QCryptographicHash>
#include <QTemporaryDir>
#include "common/garmin.h"
#include "common/textcodec.h"
#include "common/color.h"
#include "address.h"
#include "gpiparser.h"

using namespace Garmin;

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

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const TranslatedString &ts)
{
	dbg.nospace() << "TS(" << ts.lang() << ", " << ts.str() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

class CryptDevice : public QIODevice
{
public:
	CryptDevice(QIODevice *device, quint32 key, quint32 blockSize,
	  QObject *parent = 0);
	bool isSequential() const {return true;}

protected:
	qint64 readData(char *data, qint64 maxSize);
	qint64 writeData(const char *, qint64) {return -1;}

private:
	QIODevice *_device;
	quint32 _key;
	QByteArray _block;
	int _available;
};

static void demangle(quint8 *data, quint32 size, quint32 key)
{
	static const unsigned char shuf[] = {
		0xb, 0xc, 0xa, 0x0,
		0x8, 0xf, 0x2, 0x1,
		0x6, 0x4, 0x9, 0x3,
		0xd, 0x5, 0x7, 0xe
	};

	int hiCnt = 0, loCnt;
	quint8 sum = shuf[((key >> 24) + (key >> 16) + (key >> 8) + key) & 0xf];

	for (quint32 i = 0; i < size; i++) {
		quint8 hiAdd = shuf[key >> (hiCnt << 2) & 0xf] + sum;
		loCnt = (hiCnt > 6) ? 0 : hiCnt + 1;
		quint8 loAdd = shuf[key >> (loCnt << 2) & 0xf] + sum;
		quint8 hi = data[i] - (hiAdd << 4);
		quint8 lo = data[i] - loAdd;
		data[i] = (hi & 0xf0) | (lo & 0x0f);
		hiCnt = (loCnt > 6) ? 0 : loCnt + 1;
	}
}

CryptDevice::CryptDevice(QIODevice *device, quint32 key, quint32 blockSize,
  QObject *parent) : QIODevice(parent), _device(device), _key(key), _available(0)
{
	_block.resize(blockSize);
	setOpenMode(_device->openMode());
}

qint64 CryptDevice::readData(char *data, qint64 maxSize)
{
	qint64 rs, ts = 0;
	int cs;

	if (_available) {
		cs = qMin(maxSize, (qint64)_available);
		memcpy(data, _block.constData() + _block.size() - _available, cs);
		_available -= cs;
		maxSize -= cs;
		ts = cs;
	}

	while (maxSize) {
		if ((rs = _device->read(_block.data(), _block.size())) < 0)
			return -1;
		else if (!rs)
			break;
		_available = rs;
		demangle((quint8*)_block.data(), _available, _key);
		cs = qMin(maxSize, (qint64)_available);
		memcpy(data + ts, _block.constData(), cs);
		_available -= cs;
		maxSize -= cs;
		ts += cs;
	}

	return ts;
}


class DataStream : public QDataStream
{
public:
	DataStream(QIODevice *d) : QDataStream(d) {}

	void setCodepage(quint16 codepage) {_codec = TextCodec(codepage);}

	quint16 readString(QString &str);
	qint32 readInt24();
	quint8 readRecordHeader(RecordHeader &hdr);
	quint32 readTranslatedObjects(QList<TranslatedString> &objects);

	quint32 skipRecord();
	quint16 nextHeaderType();

private:
	TextCodec _codec;
};

quint16 DataStream::readString(QString &str)
{
	quint16 len;
	*this >> len;
	QByteArray ba;
	ba.resize(len);
	readRawData(ba.data(), len);
	str = _codec.toString(ba);

	return len + 2;
}

qint32 DataStream::readInt24()
{
	unsigned char data[3];
	quint32 val;

	readRawData((char*)data, sizeof(data));
	val = data[0] | ((quint32)data[1]) << 8 | ((quint32)data[2]) << 16;
	return (val > 0x7FFFFF) ? (val & 0x7FFFFF) - 0x800000 : val;
}

quint16 DataStream::nextHeaderType()
{
	quint16 type = 0;

	if (device()->peek((char*)&type, sizeof(type)) < (qint64)sizeof(type)) {
		setStatus(QDataStream::ReadCorruptData);
		return 0xFFFF;
	} else
		return qFromLittleEndian(type);
}

quint8 DataStream::readRecordHeader(RecordHeader &hdr)
{
	*this >> hdr.type >> hdr.flags >> hdr.size;
	if (hdr.flags & 0xA)
		*this >> hdr.extra;
	return (hdr.flags & 0xA) ? 12 : 8;
}

quint32 DataStream::skipRecord()
{
	RecordHeader rh;
	quint8 rs = readRecordHeader(rh);
	skipRawData(rh.size);

	return rs + rh.size;
}

quint32 DataStream::readTranslatedObjects(QList<TranslatedString> &objects)
{
	qint32 size = 0, ret;
	char lang[3];

	memset(lang, 0, sizeof(lang));
	objects.clear();

	*this >> size;
	ret = size + 4;
	while (status() == QDataStream::Ok && size > 0) {
		QString str;
		readRawData(lang, sizeof(lang) - 1);
		size -= readString(str) + 2;
		objects.append(TranslatedString(lang, str));
	}

	if (size < 0)
		setStatus(QDataStream::ReadCorruptData);

	return ret;
}


static quint32 readFprsRecord(DataStream &stream)
{
	RecordHeader rh;
	quint16 s1;
	quint8 rs, s2, s3, s4;

	rs = stream.readRecordHeader(rh);
	stream >> s1 >> s2 >> s3 >> s4;

	return rs + 5;
}

static quint32 readFileDataRecord(DataStream &stream)
{
	RecordHeader rh;
	quint32 ds, flags;
	quint16 s2, s3;
	quint8 rs;
	QList<TranslatedString> obj;

	rs = stream.readRecordHeader(rh);
	stream >> flags >> s2 >> s3;
	ds = 8;
	// name
	ds += stream.readTranslatedObjects(obj);
	// copyright
	ds += stream.readTranslatedObjects(obj);
	// additional stuff depending on flags
	// ...

	if (ds > rh.size)
		stream.setStatus(QDataStream::ReadCorruptData);
	else if (ds < rh.size)
		stream.skipRawData(rh.size - ds);

	return rs + rh.size;
}

static quint32 readDescription(DataStream &stream, Waypoint &waypoint)
{
	RecordHeader rh;
	quint8 rs;
	quint32 ds;
	QList<TranslatedString> obj;

	rs = stream.readRecordHeader(rh);
	ds = stream.readTranslatedObjects(obj);
	if (!obj.isEmpty())
		waypoint.setDescription(obj.first().str());

	if (ds != rh.size)
		stream.setStatus(QDataStream::ReadCorruptData);

	return rs + rh.size;
}

static quint32 readNotes(DataStream &stream, Waypoint &waypoint)
{
	RecordHeader rh;
	quint8 rs, flags;
	quint32 ds = 1;

	rs = stream.readRecordHeader(rh);
	stream >> flags;
	if (flags & 0x1) {
		QList<TranslatedString> obj;
		ds += stream.readTranslatedObjects(obj);
		if (!obj.isEmpty())
			waypoint.setComment(obj.first().str());
	}
	if (flags & 0x2) {
		QString str;
		ds += stream.readString(str);
		if (!str.isEmpty())
			waypoint.setComment(str);
	}

	if (ds != rh.size)
		stream.setStatus(QDataStream::ReadCorruptData);

	return rs + rh.size;
}

static quint32 readContact(DataStream &stream, Waypoint &waypoint)
{
	RecordHeader rh;
	quint8 rs;
	quint16 flags;
	quint32 ds = 2;
	QString str;
	QList<TranslatedString> obj;

	rs = stream.readRecordHeader(rh);
	stream >> flags;

	if (flags & 0x1) {
		ds += stream.readString(str);
		waypoint.setPhone(str);
	}
	if (flags & 0x2) // phone2
		ds += stream.readString(str);
	if (flags & 0x4) // fax
		ds += stream.readString(str);
	if (flags & 0x8) {
		ds += stream.readString(str);
		waypoint.addLink(Link("mailto:" + str, str));
	}
	if (flags & 0x10) {
		ds += stream.readString(str);
		QUrl url(str);
		waypoint.addLink(Link(url.scheme().isEmpty()
		  ? "http://" + str : str, str));
	}
	if (flags & 0x20) // unknown
		ds += stream.readTranslatedObjects(obj);

	if (ds != rh.size)
		stream.setStatus(QDataStream::ReadCorruptData);

	return rs + rh.size;
}

static quint32 readAddress(DataStream &stream, Waypoint &waypoint)
{
	RecordHeader rh;
	quint8 rs;
	quint16 flags;
	quint32 ds = 2;
	QList<TranslatedString> obj;
	QString str;
	Address addr;

	rs = stream.readRecordHeader(rh);
	stream >> flags;

	if (flags & 0x1) {
		ds += stream.readTranslatedObjects(obj);
		if (!obj.isEmpty())
			addr.setCity(obj.first().str());
	}
	if (flags & 0x2) {
		ds += stream.readTranslatedObjects(obj);
		if (!obj.isEmpty())
			addr.setCountry(obj.first().str());
	}
	if (flags & 0x4) {
		ds += stream.readTranslatedObjects(obj);
		if (!obj.isEmpty())
			addr.setState(obj.first().str());
	}
	if (flags & 0x8) {
		ds += stream.readString(str);
		addr.setPostalCode(str);
	}
	if (flags & 0x10) {
		ds += stream.readTranslatedObjects(obj);
		if (!obj.isEmpty())
			addr.setStreet(obj.first().str());
	}
	if (flags & 0x20) // unknown
		ds += stream.readString(str);

	if (addr.isValid())
		waypoint.setAddress(addr.address());

	if (ds != rh.size)
		stream.setStatus(QDataStream::ReadCorruptData);

	return rs + rh.size;
}

static const QTemporaryDir &tempDir()
{
	static QTemporaryDir dir;
	return dir;
}

static quint32 readImageInfo(DataStream &stream, Waypoint &waypoint,
  const QString &fileName, int &imgId)
{
	RecordHeader rh;
	quint8 rs, s1;
	quint32 size;

	rs = stream.readRecordHeader(rh);
	stream >> s1 >> size;

	QByteArray ba;
	ba.resize(size);
	stream.readRawData(ba.data(), ba.size());

	if (tempDir().isValid()) {
		QBuffer buf(&ba);
		QImageReader ir(&buf);

		QByteArray id(fileName.toUtf8() + QByteArray::number(imgId++));
		QFile imgFile(tempDir().path() + "/" + QString("%0.%1").arg(
		  QCryptographicHash::hash(id, QCryptographicHash::Sha1).toHex(),
		  QString(ir.format())));
		imgFile.open(QIODevice::WriteOnly);
		imgFile.write(ba);
		imgFile.close();

		waypoint.addImage(imgFile.fileName());
	}

	if (size + 5 != rh.size)
		stream.setStatus(QDataStream::ReadCorruptData);

	return rs + rh.size;
}

static int speed(quint8 flags)
{
	switch (flags >> 4) {
		case 0x8:
			return 40;
		case 0x9:
			return 30;
		case 0xA:
			return 50;
		case 0xB:
			return 70;
		case 0xC:
			return 80;
		case 0xD:
			return 90;
		case 0xE:
			return 100;
		case 0xF:
			return 120;
		default:
			return 0;
	}
}

static quint32 readCamera(DataStream &stream, QVector<Waypoint> &waypoints,
  QList<Area> &polygons)
{
	RecordHeader rh;
	quint8 flags, type, s7, rs;
	qint32 top, right, bottom, left, lat, lon;
	quint32 ds = 15;


	rs = stream.readRecordHeader(rh);
	top = stream.readInt24();
	right = stream.readInt24();
	bottom = stream.readInt24();
	left = stream.readInt24();
	stream >> flags >> type >> s7;

	if (s7) {
		quint32 skip = s7 + 2 + s7/4;
		stream.skipRawData(skip);
		lat = stream.readInt24();
		lon = stream.readInt24();
		ds += skip + 6;
	} else {
		quint8 s8;
		stream.skipRawData(9);
		stream >> s8;
		quint32 skip = 3 + s8 + s8/4;
		stream.skipRawData(skip);
		lat = stream.readInt24();
		lon = stream.readInt24();
		ds += skip + 16;
	}

	waypoints.append(Coordinates(toWGS24(lon), toWGS24(lat)));

	Area area(RectC(Coordinates(toWGS24(left), toWGS24(top)),
	  Coordinates(toWGS24(right), toWGS24(bottom))));

	switch (type) {
		case 8:
			area.setDescription(QString("%1&nbsp;mi/h")
			  .arg(speed(flags)));
			break;
		case 9:
			area.setDescription(QString("%1&nbsp;km/h")
			  .arg(speed(flags)));
			break;
		case 10:
		case 11:
			area.setDescription("Red light camera");
			break;
	}

	polygons.append(area);

	if (ds > rh.size)
		stream.setStatus(QDataStream::ReadCorruptData);
	else if (ds < rh.size)
		stream.skipRawData(rh.size - ds);

	return rs + rh.size;
}

static quint32 readIconId(DataStream &stream, quint16 &id)
{
	RecordHeader rh;
	quint8 rs;

	rs = stream.readRecordHeader(rh);
	stream >> id;

	if (2 != rh.size)
		stream.setStatus(QDataStream::ReadCorruptData);

	return rs + rh.size;
}

static quint32 readPOI(DataStream &stream, QVector<Waypoint> &waypoints,
  const QString &fileName, int &imgId, QVector<QPair<int, quint16> > &icons)
{
	RecordHeader rh;
	quint8 rs;
	quint32 ds;
	qint32 lat, lon;
	quint16 s3, iconId = 0;
	QList<TranslatedString> obj;

	rs = stream.readRecordHeader(rh);
	stream >> lat >> lon >> s3;
	stream.skipRawData(s3);
	ds = 10 + s3;
	ds += stream.readTranslatedObjects(obj);

	waypoints.append(Waypoint(Coordinates(toWGS32(lon), toWGS32(lat))));
	if (!obj.isEmpty())
		waypoints.last().setName(obj.first().str());

	while (stream.status() == QDataStream::Ok && ds < rh.size) {
		switch (stream.nextHeaderType()) {
			case 4:
				ds += readIconId(stream, iconId);
				break;
			case 10:
				ds += readDescription(stream, waypoints.last());
				break;
			case 11:
				ds += readAddress(stream, waypoints.last());
				break;
			case 12:
				ds += readContact(stream, waypoints.last());
				break;
			case 13:
				ds += readImageInfo(stream, waypoints.last(), fileName, imgId);
				break;
			case 14:
				ds += readNotes(stream, waypoints.last());
				break;
			default:
				ds += stream.skipRecord();
		}
	}

	icons.append(QPair<int, quint16>(waypoints.size() - 1, iconId));

	if (ds != rh.size)
		stream.setStatus(QDataStream::ReadCorruptData);

	return rs + rh.size;
}

static quint32 readSpatialIndex(DataStream &stream, QVector<Waypoint> &waypoints,
  QList<Area> &polygons, const QString &fileName, int &imgId,
  QVector<QPair<int, quint16> > &icons)
{
	RecordHeader rh;
	quint32 ds, s5;
	qint32 top, right, bottom, left;
	quint16 s6;
	quint8 rs;

	rs = stream.readRecordHeader(rh);
	stream >> top >> right >> bottom >> left >> s5 >> s6;
	stream.skipRawData(s6);
	ds = 22 + s6;
	if (rh.flags & 0x8) {
		while (stream.status() == QDataStream::Ok && ds < rh.size) {
			switch (stream.nextHeaderType()) {
				case 2:
					ds += readPOI(stream, waypoints, fileName, imgId, icons);
					break;
				case 8:
					ds += readSpatialIndex(stream, waypoints, polygons,
					  fileName, imgId, icons);
					break;
				case 19:
					ds += readCamera(stream, waypoints, polygons);
					break;
				default:
					ds += stream.skipRecord();
			}
		}
	}

	if (ds != rh.size)
		stream.setStatus(QDataStream::ReadCorruptData);

	return rs + rh.size;
}

static quint32 readSymbol(DataStream &stream, QPixmap &pixmap)
{
	RecordHeader rh;
	quint8 rs, u8, bpp, transparent;
	quint32 ds = 36, u32, imageSize, paletteSize, bgColor;
	quint16 id, height, width, lineSize;
	QByteArray data;
	QVector<QRgb> palette;
	QImage img;

	rs = stream.readRecordHeader(rh);
	stream >> id >> height >> width >> lineSize >> bpp >> u8 >> u8 >> u8
	  >> imageSize >> u32 >> paletteSize >> bgColor >> transparent >> u8 >> u8
	  >> u8 >> u32;
	if (imageSize) {
		data.resize(imageSize);
		stream.readRawData(data.data(), data.size());
		ds += data.size();
	}
	if (paletteSize) {
		quint32 rgb;
		palette.resize(paletteSize);
		for (quint32 i = 0; i < paletteSize; i++) {
			stream >> rgb;
			palette[i] = (transparent && rgb == bgColor)
			  ? QRgb(0)
			  : Color::rgb((rgb & 0x000000FF), (rgb & 0x0000FF00) >> 8,
			    (rgb & 0x00FF0000) >> 16);
		}
		ds += paletteSize * 4;
	}

	if (data.size() >= lineSize * height) {
		if (paletteSize) {
			img = QImage((uchar*)data.data(), width, height, lineSize,
			  QImage::Format_Indexed8);
			img.setColorTable(palette);
		} else
			img = QImage((uchar*)data.data(), width, height, lineSize,
			  QImage::Format_RGBX8888).rgbSwapped();
	}
	pixmap = QPixmap::fromImage(img);

	/* There should be no more data left in the record, but broken GPI files
	   generated by pinns.co.uk tools exist in the wild so we read out
	   the record as a workaround for such files. */
	if (ds > rh.size)
		stream.setStatus(QDataStream::ReadCorruptData);
	else if (ds < rh.size)
		stream.skipRawData(rh.size - ds);

	return rs + rh.size;
}

static void readPOIDatabase(DataStream &stream, QVector<Waypoint> &waypoints,
  QList<Area> &polygons, const QString &fileName, int &imgId)
{
	RecordHeader rh;
	QList<TranslatedString> obj;
	quint32 ds;
	QVector<QPair<int, quint16> > il;
	QVector<QPixmap> icons;

	stream.readRecordHeader(rh);
	ds = stream.readTranslatedObjects(obj);
	ds += readSpatialIndex(stream, waypoints, polygons, fileName, imgId, il);
	if (rh.flags & 0x8) {
		while (stream.status() == QDataStream::Ok && ds < rh.size) {
			switch (stream.nextHeaderType()) {
				case 5:
					icons.append(QPixmap());
					ds += readSymbol(stream, icons.last());
					break;
				case 8:
					ds += readSpatialIndex(stream, waypoints, polygons,
					  fileName, imgId, il);
					break;
				default:
					ds += stream.skipRecord();
			}
		}
	}

	for (int i = 0; i < il.size(); i++) {
		const QPair<int, quint16> &e = il.at(i);
		if (e.second < icons.size())
			waypoints[e.first].setIcon(icons.at(e.second));
	}

	if (ds != rh.size)
		stream.setStatus(QDataStream::ReadCorruptData);
}

bool GPIParser::readData(DataStream &stream, QVector<Waypoint> &waypoints,
  QList<Area> &polygons, const QString &fileName)
{
	int imgId = 0;

	while (stream.status() == QDataStream::Ok) {
		switch (stream.nextHeaderType()) {
			case 0x09:
				readPOIDatabase(stream, waypoints, polygons, fileName,
				  imgId);
				break;
			case 0xffff: // EOF
				stream.skipRecord();
				if (stream.status() == QDataStream::Ok)
					return true;
				break;
			default:
				stream.skipRecord();
		}
	}

	_errorString = "Invalid/corrupted GPI data";

	return false;
}

bool GPIParser::readGPIHeader(DataStream &stream)
{
	RecordHeader rh;
	char m1[6], m2[2];
	quint16 codepage = 0;
	quint8 s2, s3;
	quint32 ds;

	stream.readRecordHeader(rh);
	stream.readRawData(m1, sizeof(m1));
	stream.readRawData(m2, sizeof(m2));
	stream >> codepage >> s2 >> s3;
	ds = sizeof(m1) + sizeof(m2) + 4;

	stream.setCodepage(codepage);

	if (s2 & 0x10)
		ds += readFileDataRecord(stream);

	if (stream.status() != QDataStream::Ok || ds != rh.size) {
		_errorString = "Invalid GPI header";
		return false;
	} else
		return true;
}

bool GPIParser::readFileHeader(DataStream &stream, quint32 &ebs)
{
	RecordHeader rh;
	quint32 ds, s7;
	quint16 s10;
	quint8 s5, s6, s8, s9;
	char magic[6];

	stream.readRecordHeader(rh);
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

	ebs = (s8 & 0x4) ? s9 * 8 + 8 : 0;

	if (stream.status() != QDataStream::Ok || ds != rh.size) {
		_errorString = "Invalid file header";
		return false;
	} else
		return true;
}

bool GPIParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons, QVector<Waypoint> &waypoints)
{
	Q_UNUSED(tracks);
	Q_UNUSED(routes);
	DataStream stream(file);
	quint32 ebs;

	stream.setByteOrder(QDataStream::LittleEndian);

	if (!readFileHeader(stream, ebs) || !readGPIHeader(stream))
		return false;

	if (ebs) {
		CryptDevice dev(stream.device(), 0xf870b5, ebs);
		DataStream cryptStream(&dev);
		cryptStream.setByteOrder(QDataStream::LittleEndian);
		return readData(cryptStream, waypoints, polygons, file->fileName());
	} else
		return readData(stream, waypoints, polygons, file->fileName());
}
