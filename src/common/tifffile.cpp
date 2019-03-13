#include "tifffile.h"

#define TIFF_II     0x4949
#define TIFF_MM     0x4D4D
#define TIFF_MAGIC  42

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
	if (magic != TIFF_MAGIC)
		return;
	if (!readValue(_ifd))
		return;
}
