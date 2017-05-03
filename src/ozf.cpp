#include <QtEndian>
#include <QFile>
#include "ozf.h"


#define OZF2_MAGIC  0x7778
#define OZF3_MAGIC  0x7780
#define SEPARATOR   0x77777777

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

bool OZF::read(void *data, size_t size)
{
	if (_file.read((char*)data, size) < (qint64)size)
		return false;

	if (_decrypt)
		decrypt(data, size, _key);

	return true;
}

bool OZF::readKey()
{
	quint8 randomNumber, initial;
	quint32 keyblock;


	if (!_file.seek(14))
		return false;
	if (!readValue(randomNumber))
		return false;

	if (!_file.seek(162))
		return false;
	if (!readValue(initial))
		return false;

	_decrypt = true; _key = initial;
	if (!_file.seek(15 + randomNumber))
		return false;
	if (!readValue(keyblock))
		return false;

	switch (keyblock & 0xFF) {
		case 0xf1:
			initial += 0x8a;
			break;
		case 0x18:
		case 0x54:
			initial += 0xa0;
			break;
		case 0x56:
			initial += 0xb9;
			break;
		case 0x43:
			initial += 0x6a;
			break;
		case 0x83:
			initial += 0xa4;
			break;
		case 0xc5:
			initial += 0x7e;
			break;
		case 0x38:
			initial += 0xc1;
			break;
		default:
			break;
	}

	_key = initial;

	return true;
}

bool OZF::readHeaders()
{
	quint16 magic;
	quint32 separator;

	if (!readValue(magic))
		return false;

	if (magic == OZF2_MAGIC) {
		if (!_file.seek(_file.pos() + 52))
			return false;
		if (!readValue(separator) || separator != SEPARATOR)
			return false;
	} else if (magic == OZF3_MAGIC) {
		if (!readKey())
			return false;
	} else
		return false;

	return true;
}

bool OZF::readTileTable()
{
	quint32 offset, bgr0, w, h;
	quint16 x, y;


	if (!_file.seek(_file.size() - 4))
		return false;
	// table offset
	if (!readValue(offset))
		return false;
	if (!_file.seek(offset))
		return false;
	// tiles offset (zoom level 0)
	if (!readValue(offset))
		return false;
	if (!_file.seek(offset))
		return false;

	if (!readValue(w))
		return false;
	if (!readValue(h))
		return false;
	if (!readValue(x))
		return false;
	if (!readValue(y))
		return false;

	_size = QSize(w, h);
	_dim = QSize(x, y);

	_palette = QVector<quint32>(256);
	if (!read(&(_palette[0]), sizeof(quint32) * 256))
		return false;
	for (int i = 0; i < _palette.size(); i++) {
		bgr0 = qFromLittleEndian(_palette.at(i));

		quint32 b = (bgr0 & 0x000000FF);
		quint32 g = (bgr0 & 0x0000FF00) >> 8;
		quint32 r = (bgr0 & 0x00FF0000) >> 16;

		_palette[i] = 0xFF000000 | r << 16 | g << 8 | b;
	}

	_tiles = QVector<quint32>(_dim.width() * _dim.height() + 1);
	for (int i = 0; i < _tiles.size(); i++)
		if (!readValue(_tiles[i]))
			return false;

	return true;
}

bool OZF::load(const QString &path)
{
	if (_file.isOpen())
		_file.close();

	_file.setFileName(path);
	if (!_file.open(QIODevice::ReadOnly))
		return false;

	if (!readHeaders()) {
		qWarning("%s: not a OZF2/OZF3 file", qPrintable(_file.fileName()));
		_file.close();
		return false;
	}

	if (!readTileTable()) {
		qWarning("%s: file format error", qPrintable(_file.fileName()));
		_file.close();
		_size = QSize();
		return false;
	}

	return true;
}

QPixmap OZF::tile(int x, int y)
{
	Q_ASSERT(_file.isOpen());

	int i = (y/tileSize().height()) * _dim.width() + (x/tileSize().width());
	if (i >= _tiles.size() - 1 || i < 0)
		return QPixmap();

	int size = _tiles.at(i+1) - _tiles.at(i);
	if (!_file.seek(_tiles.at(i)))
		return QPixmap();

	QByteArray ba = _file.read(size);
	if (ba.size() != size)
		return QPixmap();
	if (_decrypt)
		decrypt(ba.data(), qMin(16, ba.size()), _key);
	quint32 bes = qToBigEndian(tileSize().width() * tileSize().height());
	ba.prepend(QByteArray((char*)&bes, sizeof(bes)));
	QByteArray uba = qUncompress(ba);
	if (uba.size() != tileSize().width() * tileSize().height())
		return QPixmap();

	QImage img((const uchar*)uba.constData(), tileSize().width(),
	  tileSize().height(), QImage::Format_Indexed8);
	img.setColorTable(_palette);

	return QPixmap::fromImage(img.mirrored());
}
