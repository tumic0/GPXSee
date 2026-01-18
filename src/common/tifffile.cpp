#include "tifffile.h"

#define TIFF_II        0x4949
#define TIFF_MM        0x4D4D
#define TIFF_MAGIC     42
#define TIFF_MAGIC_BIG 43

TIFFFile::TIFFFile(QIODevice *device) : _device(device), _ifd(0), _offset(0)
{
	quint16 endian, magic;

	_offset = _device->pos();

	if (_device->read((char*)&endian, sizeof(endian)) < (qint64)sizeof(endian))
		return;
	if (endian == TIFF_II)
		_be = false;
	else if (endian == TIFF_MM)
		_be = true;
	else
		return;

	if (!readValue(magic))
		return;
	if (magic == TIFF_MAGIC_BIG)
		_big = true;
	else if (magic == TIFF_MAGIC)
		_big = false;
	else
		return;

	if (_big) {
		quint16 os, zero;
		if (!(readValue(os) && readValue(zero)))
			return;
		if (!(os == 8 && zero == 0))
			return;
		if (!readValue(_ifd))
			return;
	} else {
		quint32 ifd;
		if (!readValue(ifd))
			return;
		_ifd = ifd;
	}
}
