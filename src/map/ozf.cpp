#include <cstring>
#include <QtEndian>
#include <QFile>
#include "ozf.h"


#define OZF2_MAGIC  0x7778
#define OZF3_MAGIC  0x7780

static const quint8 XKEY[] =
{
	0x2D, 0x4A, 0x43, 0xF1, 0x27, 0x9B, 0x69, 0x4F,
	0x36, 0x52, 0x87, 0xEC, 0x5F, 0x42, 0x53, 0x22,
	0x9E, 0x8B, 0x2D, 0x83, 0x3D, 0xD2, 0x84, 0xBA,
	0xD8, 0x5B
};

static void decrypt(void *data, size_t size, quint8 init)
{
	for (size_t i = 0; i < size; i++)
		reinterpret_cast<quint8*>(data)[i] ^= XKEY[i % sizeof(XKEY)] + init;
}

template<class T> bool OZF::readValue(T &val)
{
	T data;

	if (_file.read((char*)&data, sizeof(T)) < (qint64)sizeof(T))
		return false;

	if (_decrypt)
		decrypt(&data, sizeof(T), _key);

	if (sizeof(T) > 1)
		val = qFromLittleEndian(data);
	else
		val = data;

	return true;
}

bool OZF::read(void *data, size_t size, size_t decryptSize)
{
	if (_file.read((char*)data, size) < (qint64)size)
		return false;

	if (_decrypt)
		decrypt(data, decryptSize ? qMin(decryptSize, size) : size, _key);

	return true;
}

bool OZF::initOZF3()
{
	quint8 randomNumber, initial;
	quint8 h1[8];
	quint8 h2[16], h2d[16];


	if (!_file.seek(14))
		return false;
	if (!readValue(randomNumber))
		return false;

	if (!_file.seek(162))
		return false;
	if (!readValue(initial))
		return false;

	_decrypt = true;
	_key = initial;

	if (!_file.seek(0))
		return false;
	if (!read(h1, sizeof(h1)))
		return false;
	_tileSize = *(h1 + 6);

	if (!_file.seek(15 + randomNumber + 4))
		return false;
	if (_file.read((char*)h2, sizeof(h2)) != (qint64)sizeof(h2))
		return false;

	for (int i = 0; i < 256; i++) {
		memcpy(h2d, h2, sizeof(h2d));
		decrypt(h2d, sizeof(h2d), (quint8)i);

		if ((quint32)*h2d == 40 && (quint16)*(h2d + 12) == 1
		  && (quint16)*(h2d + 14) == 8) {
			_key = (quint8)i;
			return true;
		}
	}

	return false;
}

bool OZF::initOZF2()
{
	if (!_file.seek(6))
		return false;
	if (!readValue(_tileSize))
		return false;

	return true;
}

bool OZF::readHeaders()
{
	quint16 magic;

	if (!readValue(magic))
		return false;

	if (magic == OZF2_MAGIC) {
		if (!initOZF2())
			return false;
	} else if (magic == OZF3_MAGIC) {
		if (!initOZF3())
			return false;
	} else {
		qWarning("%s: not a OZF2/OZF3 file", qPrintable(_file.fileName()));
		return false;
	}

	return true;
}

bool OZF::readTileTable()
{
	quint32 tableOffset, headerOffset, bgr0, w, h;
	quint16 x, y;
	int zooms;


	if (!_file.seek(_file.size() - sizeof(tableOffset)))
		return false;
	if (!readValue(tableOffset))
		return false;
	zooms = (int)((_file.size() - tableOffset - sizeof(quint32))
	  / sizeof(quint32));

	for (int i = 0; i < zooms - 2; i++) {
		if (!_file.seek(tableOffset + i * sizeof(quint32)))
			return false;
		if (!readValue(headerOffset))
			return false;
		if (!_file.seek(headerOffset))
			return false;

		if (!readValue(w))
			return false;
		if (!readValue(h))
			return false;
		if (!readValue(x))
			return false;
		if (!readValue(y))
			return false;

		Zoom zoom;
		zoom.size = QSize(w, h);
		zoom.dim = QSize(x, y);

		zoom.palette = QVector<quint32>(256);
		if (!read(&(zoom.palette[0]), sizeof(quint32) * 256))
			return false;
		for (int i = 0; i < zoom.palette.size(); i++) {
			bgr0 = qFromLittleEndian(zoom.palette.at(i));

			quint32 b = (bgr0 & 0x000000FF);
			quint32 g = (bgr0 & 0x0000FF00) >> 8;
			quint32 r = (bgr0 & 0x00FF0000) >> 16;

			zoom.palette[i] = 0xFF000000 | r << 16 | g << 8 | b;
		}

		zoom.tiles = QVector<quint32>(zoom.dim.width() * zoom.dim.height() + 1);
		for (int i = 0; i < zoom.tiles.size(); i++)
			if (!readValue(zoom.tiles[i]))
				return false;

		_zooms.append(zoom);
	}

	return _zooms.isEmpty() ? false : true;
}

bool OZF::open()
{
	if (!_file.open(QIODevice::ReadOnly))
		return false;

	if (!readHeaders()) {
		qWarning("%s: Invalid header", qPrintable(_file.fileName()));
		_file.close();
		return false;
	}

	if (!readTileTable()) {
		qWarning("%s: Invalid tile table", qPrintable(_file.fileName()));
		_file.close();
		return false;
	}

	return true;
}

QPixmap OZF::tile(int zoom, int x, int y)
{
	Q_ASSERT(_file.isOpen());
	Q_ASSERT(0 <= zoom && zoom < _zooms.count());

	const Zoom &z = _zooms.at(zoom);

	int i = (y/tileSize().height()) * z.dim.width() + (x/tileSize().width());
	if (i >= z.tiles.size() - 1 || i < 0)
		return QPixmap();

	int size = z.tiles.at(i+1) - z.tiles.at(i);
	if (!_file.seek(z.tiles.at(i)))
		return QPixmap();

	quint32 bes = qToBigEndian(tileSize().width() * tileSize().height());
	QByteArray ba;
	ba.resize(sizeof(bes) + size);
	memcpy(ba.data(), &bes, sizeof(bes));

	if (!read(ba.data() + sizeof(bes), size, 16))
		return QPixmap();
	QByteArray uba = qUncompress(ba);
	if (uba.size() != tileSize().width() * tileSize().height())
		return QPixmap();

	QImage img((const uchar*)uba.constData(), tileSize().width(),
	  tileSize().height(), QImage::Format_Indexed8);
	img.setColorTable(z.palette);

	return QPixmap::fromImage(img.mirrored());
}

QSize OZF::size(int zoom) const
{
	Q_ASSERT(_file.isOpen());
	Q_ASSERT(0 <= zoom && zoom < _zooms.count());

	return _zooms.at(zoom).size;
}

QPointF OZF::scale(int zoom) const
{
	return QPointF((qreal)size(zoom).width() / (qreal)size(0).width(),
	  (qreal)size(zoom).height() / (qreal)size(0).height());
}

bool OZF::isOZF(const QString &path)
{
	QFile file(path);
	quint16 magic;

	if (!file.open(QIODevice::ReadOnly))
		return false;
	if (file.read((char*)&magic, sizeof(magic)) < (qint64)sizeof(magic))
		return false;

	magic = qFromLittleEndian(magic);
	if (magic == OZF2_MAGIC || magic == OZF3_MAGIC)
		return true;

	return false;
}
