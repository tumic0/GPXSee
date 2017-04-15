#include <QtEndian>
#include <QFile>
#include "ozf.h"


#define OZF2_MAGIC     0x7778
#define OZF2_SEPARATOR 0x77777777

template<class T> bool OZF::readValue(T &val)
{
	T data;

	if (_file.read((char*)&data, sizeof(T)) < (qint64)sizeof(T))
		return false;

	if (sizeof(T) > 1)
		val = qFromLittleEndian(data);
	else
		val = data;

	return true;
}

bool OZF::readHeaders()
{
	quint16 magic;
	quint32 separator;

	if (!readValue(magic) || magic != OZF2_MAGIC)
		return false;

	if (!_file.seek(_file.pos() + 52))
		return false;
	if (!readValue(separator) || separator != OZF2_SEPARATOR)
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
	for (int i = 0; i < _palette.size(); i++) {
		if (!readValue(bgr0))
			return false;

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
		qWarning("%s: not a OZF2 file", qPrintable(_file.fileName()));
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
