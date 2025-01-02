#include <cstring>
#include <QDataStream>
#include <QPixmapCache>
#include <QPainter>
#include "common/util.h"
#include "common/color.h"
#include "qctmap.h"

#define TILE_SIZE 64
#define TILE_PIXELS (TILE_SIZE * TILE_SIZE)
#define MAGIC 0x1423D5FF

static quint8 bpp(quint8 colours)
{
	if (colours <= 2)
		return 1;
	if (colours <= 4)
		return 2;
	if (colours <= 8)
		return 3;
	if (colours <= 16)
		return 4;
	if (colours <= 32)
		return 5;
	if (colours <= 64)
		return 6;
	if (colours <= 128)
		return 7;

	return 8;
}

static bool validateTable(const QVector<quint8> &table)
{
	int delta;

	for (int i = 0; i < table.size(); i++) {
		if (table.at(i) == 128) {
			if (i + 2 >= table.size())
				return false;
			delta = 65537 - (256 * table.at(i+2) + table.at(i+1)) + 2;
			if (i + delta >= table.size())
				return false;
			i += 2;
		} else if (table.at(i) > 128) {
			delta = 257 - table.at(i);
			if (i + delta >= table.size())
				return false;
		}
	}

	return true;
}

static bool createTable(QDataStream &stream, QVector<quint8> &table)
{
	int idx = 0;
	int colours = 0;
	int branches = 0;

	table.reserve(256);

	while (stream.status() == QDataStream::Ok && colours <= branches) {
		table.resize(table.size() + 1);
		stream >> table[idx];

		if (table[idx] == 128) {
			table.resize(table.size() + 2);
			stream >> table[++idx];
			stream >> table[++idx];
			branches++;
		} else if (table[idx] > 128)
			branches++;
		else
			colours++;

		idx++;
	}

	return (stream.status() == QDataStream::Ok);
}

static bool huffman(QDataStream &stream, quint8 tileData[TILE_PIXELS])
{
	QVector<quint8> table;
	if (!createTable(stream, table))
		return false;

	if (table.size() == 1) {
		memset(tileData, table[0], TILE_PIXELS);
	} else {
		if (!validateTable(table))
			return false;

		const quint8 *tp = table.constData();
		int bitsLeft = 8;
		int bitVal;
		quint8 val;

		stream >> val;

		for (int pixelnum = 0; pixelnum < TILE_PIXELS; ) {
			if (*tp < 128) {
				tileData[pixelnum++] = *tp;
				tp = table.constData();
			} else {
				bitVal = (val & 1);

				val >>= 1;
				bitsLeft--;
				if (bitsLeft == 0) {
					stream >> val;
					bitsLeft = 8;
				}

				if (bitVal == 0) {
					if (*tp == 128)
						tp += 2;
					tp++;
				} else {
					if (*tp > 128)
						tp += 257 - (*tp);
					else if (*tp == 128)
						tp += 65537 - (256 * tp[2] + tp[1]) + 2;
				}
			}
		}
	}

	return (stream.status() == QDataStream::Ok);
}

static bool pixelPacking(QDataStream &stream, quint8 tileData[TILE_PIXELS],
  quint8 colours)
{
	quint8 shift = bpp(colours);
	quint32 mask = (1 << shift) - 1;
	int wordSize = 32 / shift;
	quint8 paletteIndex[256];

	for (quint8 i = 0; i < colours; i++)
		stream >> paletteIndex[i];

	for (int pixelnum = 0; pixelnum < TILE_PIXELS; ) {
		quint32 colour, val;
		stream >> val;

		for (int runs = 0; runs < wordSize; runs++) {
			colour = val & mask;
			val = val >> shift;
			tileData[pixelnum++] = paletteIndex[colour];
		}
	}

	return (stream.status() == QDataStream::Ok);
}

static bool rle(QDataStream &stream, quint8 tileData[TILE_PIXELS],
  quint8 colours)
{
	quint8 bits = bpp(colours);
	quint8 paletteMask = (1 << bits) - 1;
	quint8 paletteIndex[256];
	quint8 val;

	for (quint8 i = 0; i < colours; i++)
		stream >> paletteIndex[i];

	for (int pixelnum = 0; pixelnum < TILE_PIXELS; ) {
		stream >> val;

		quint8 colour = val & paletteMask;
		quint8 runs = val >> bits;

		while (runs-- > 0)
			tileData[pixelnum++] = paletteIndex[colour];
	}

	return (stream.status() == QDataStream::Ok);
}

static bool readString(QDataStream &stream, quint32 offset, QString &str)
{
	char c;
	QByteArray ba;

	if (!stream.device()->seek(offset))
		return false;

	while (stream.readRawData(&c, 1) == 1) {
		if (c)
			ba.append(c);
		else {
			str = QString::fromUtf8(ba);
			return true;
		}
	}

	return false;
}

bool QCTMap::readName(QDataStream &stream)
{
	quint32 title, name;

	stream >> title >> name;
	if (stream.status() != QDataStream::Ok)
		return false;

	if (name) {
		if (!readString(stream, name, _name))
			return false;
	} else if (title) {
		if (!readString(stream, title, _name))
			return false;
	} else
		_name = Util::file2name(path());

	return true;
}

bool QCTMap::readSize(QDataStream &stream)
{
	stream >> _cols >> _rows;
	return (stream.status() == QDataStream::Ok);
}

bool QCTMap::readDatumShift(QDataStream &stream)
{
	quint32 ext, shift;

	if (!stream.device()->seek(0x54))
		return false;
	stream >> ext;
	if (stream.status() != QDataStream::Ok)
		return false;
	if (!ext)
		return true;
	if (!stream.device()->seek(ext + 4))
		return false;
	stream >> shift;
	if (stream.status() != QDataStream::Ok)
		return false;
	if (!shift)
		return true;
	if (!stream.device()->seek(shift))
		return false;
	stream >> _shiftN >> _shiftE;

	return (stream.status() == QDataStream::Ok);
}

bool QCTMap::readHeader(QDataStream &stream)
{
	quint32 magic, version;
	stream >> magic >> version;

	if (stream.status() != QDataStream::Ok || magic != MAGIC) {
		_errorString = "Not a QCT map";
		return false;
	}
	if (version == 0x20000001) {
		_errorString = "QC3 files not supported";
		return false;
	}
	if (!readSize(stream)) {
		_errorString = "Error reading map dimensions";
		return false;
	}
	if (!readName(stream)) {
		_errorString = "Error reading map name";
		return false;
	}
	if (!readDatumShift(stream)) {
		_errorString = "Error reading datum shift";
		return false;
	}

	return true;
}

bool QCTMap::readGeoRef(QDataStream &stream)
{
	if (!stream.device()->seek(0x60))
		return false;

	stream >> _eas >> _easY >> _easX >> _easYY >> _easXY >> _easXX >> _easYYY
	  >> _easYYX >> _easXXY >> _easXXX >> _nor >> _norY >> _norX >> _norYY
	  >> _norXY >> _norXX >> _norYYY >> _norYYX >> _norXXY >> _norXXX;
	stream >> _lat >> _latX >> _latY >> _latXX >> _latXY >> _latYY >> _latXXX
	  >> _latXXY >> _latXYY >> _latYYY >> _lon >> _lonX >> _lonY >> _lonXX
	  >> _lonXY >> _lonYY >> _lonXXX >> _lonXXY >> _lonXYY >> _lonYYY;

	return (stream.status() == QDataStream::Ok);
}

bool QCTMap::readPalette(QDataStream &stream)
{
	if (!stream.device()->seek(0x01A0))
		return false;

	_palette.resize(256);

	quint32 bgr;
	for (int i = 0; i < _palette.size(); i++) {
		stream >> bgr;
		_palette[i] = Color::bgr2rgb(bgr);
	}

	return (stream.status() == QDataStream::Ok);
}

bool QCTMap::readIndex(QDataStream &stream)
{
	if (!stream.device()->seek(0x45A0))
		return false;

	_index.resize(_cols * _rows);
	for (int i = 0; i < _cols * _rows; i++)
		stream >> _index[i];

	return (stream.status() == QDataStream::Ok);
}

QCTMap::QCTMap(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _file(fileName), _shiftE(0), _shiftN(0),
  _mapRatio(1.0), _valid(false)
{
	if (!_file.open(QIODevice::ReadOnly)) {
		_errorString = _file.errorString();
		return;
	}

	QDataStream stream(&_file);
	stream.setByteOrder(QDataStream::LittleEndian);

	if (!readHeader(stream))
		return;
	if (!readGeoRef(stream)) {
		_errorString = "Error reading georeference info";
		return;
	}
	if (!readPalette(stream)) {
		_errorString = "Error reading colour palette";
		return;
	}
	if (!readIndex(stream)) {
		_errorString = "Error reading tile index";
		return;
	}

	_file.close();

	_valid = true;
}

void QCTMap::load(const Projection &in, const Projection &out,
  qreal deviceRatio, bool hidpi)
{
	Q_UNUSED(in);
	Q_UNUSED(out);

	_mapRatio = hidpi ? deviceRatio : 1.0;
	if (!_file.open(QIODevice::ReadOnly))
		qWarning("%s: %s", qPrintable(_file.fileName()),
		  qPrintable(_file.errorString()));
}

void QCTMap::unload()
{
	_file.close();
}

QRectF QCTMap::bounds()
{
	return QRectF(QPointF(0, 0), QSizeF(_cols * TILE_SIZE, _rows * TILE_SIZE)
	  / _mapRatio);
}

QPointF QCTMap::ll2xy(const Coordinates &c)
{
	double lon = c.lon() - _shiftE;
	double lon2 = lon * lon;
	double lon3 = lon2 * lon;
	double lat = c.lat() - _shiftN;
	double lat2 = lat * lat;
	double lat3 = lat2 * lat;

	double x = _easXXX*lon3 + _easXX*lon2 + _easX*lon + _easYYY*lat3
	  + _easYY*lat2 + _easY*lat + _easXXY*lon2*lat
	  + _easYYX*lat2*lon + _easXY*lon*lat + _eas;
	double y = _norXXX*lon3 + _norXX*lon2 + _norX*lon + _norYYY*lat3
	  + _norYY*lat2 + _norY*lat + _norXXY*lon2*lat
	  + _norYYX*lat2*lon + _norXY*lon*lat + _nor;

	return QPointF(x - _shiftE, y - _shiftN) / _mapRatio;
}

Coordinates QCTMap::xy2ll(const QPointF &p)
{
	qreal x = p.x() * _mapRatio;
	qreal x2 = x * x;
	qreal x3 = x2 * x;
	qreal y = p.y() * _mapRatio;
	qreal y2 = y * y;
	qreal y3 = y2 * y;

	double lon = _lon + _lonX*x + _lonY*y + _lonXX*x2
	  + _lonXY*x*y + _lonYY*y2 + _lonXXX*x3 + _lonXXY*x2*y
	  + _lonXYY*x*y2 + _lonYYY*y3;
	double lat = _lat + _latX*x + _latY*y + _latXX*x2
	  + _latXY*x*y + _latYY*y2 + _latXXX*x3 + _latXXY*x2 * y
	  + _latXYY*x*y2 + _latYYY*y3;

	return Coordinates(lon + _shiftE, lat + _shiftN);
}

QPixmap QCTMap::tile(int x, int y)
{
	static quint8 rowSeq[] = {
		 0, 32, 16, 48,  8, 40, 24, 56,  4, 36, 20, 52, 12, 44, 28, 60,
		 2, 34, 18, 50, 10, 42, 26, 58,  6, 38, 22, 54, 14, 46, 30, 62,
		 1, 33, 17, 49,  9, 41, 25, 57,  5, 37, 21, 53, 13, 45, 29, 61,
		 3, 35, 19, 51, 11, 43, 27, 59,  7, 39, 23, 55, 15, 47, 31, 63
	};
	quint8 tileData[TILE_PIXELS], imgData[TILE_PIXELS];
	quint8 packing;
	bool ret;


	if (!_file.seek(_index.at(y * _cols + x)))
		return QPixmap();

	QDataStream stream(&_file);
	stream.setByteOrder(QDataStream::LittleEndian);

	stream >> packing;
	if (stream.status() != QDataStream::Ok)
		return QPixmap();

	if (packing == 0 || packing == 255)
		ret = huffman(stream, tileData);
	else if (packing > 127)
		ret = pixelPacking(stream, tileData, 256 - packing);
	else
		ret = rle(stream, tileData, packing);

	if (!ret)
		return QPixmap();

	for (int i = 0; i < TILE_SIZE; i++)
		memcpy(imgData + i * TILE_SIZE, tileData + rowSeq[i] * TILE_SIZE,
		  TILE_SIZE);

	QImage img(imgData, TILE_SIZE, TILE_SIZE, TILE_SIZE,
	  QImage::Format_Indexed8);
	img.setColorTable(_palette);

	return QPixmap::fromImage(img);
}

void QCTMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	Q_UNUSED(flags);

	QSizeF ts(TILE_SIZE / _mapRatio, TILE_SIZE / _mapRatio);
	QPointF tl(floor(rect.left() / ts.width()) * ts.width(),
	  floor(rect.top() / ts.height()) * ts.height());

	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	for (int i = 0; i < ceil(s.width() / ts.width()); i++) {
		for (int j = 0; j < ceil(s.height() / ts.height()); j++) {
			int x = round(tl.x() * _mapRatio + i * TILE_SIZE) / TILE_SIZE;
			int y = round(tl.y() * _mapRatio + j * TILE_SIZE) / TILE_SIZE;

			QPixmap pixmap;
			QString key = path() + "/" + QString::number(x) + "_"
			  + QString::number(y);
			if (!QPixmapCache::find(key, &pixmap)) {
				pixmap = tile(x, y);
				if (!pixmap.isNull())
					QPixmapCache::insert(key, pixmap);
			}

			if (pixmap.isNull())
				qWarning("%s: error loading tile image", qPrintable(key));
			else {
				pixmap.setDevicePixelRatio(_mapRatio);
				QPointF tp(tl.x() + i * ts.width(), tl.y() + j * ts.height());
				painter->drawPixmap(tp, pixmap);
			}
		}
	}
}

Map *QCTMap::create(const QString &path, const Projection &proj, bool *isDir)
{
	Q_UNUSED(proj);

	if (isDir)
		*isDir = false;

	return new QCTMap(path);
}
