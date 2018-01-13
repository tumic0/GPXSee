#include "tifffile.h"

#define TIFF_II     0x4949
#define TIFF_MM     0x4D4D
#define TIFF_MAGIC  42

bool TIFFFile::readHeader(quint32 &ifd)
{
	quint16 endian, magic;

	if (QFile::read((char*)&endian, sizeof(endian)) < (qint64)sizeof(endian))
		return false;
	if (endian == TIFF_II)
		_be = false;
	else if (endian == TIFF_MM)
		_be = true;
	else
		return false;

	if (!readValue(magic))
		return false;
	if (magic != TIFF_MAGIC)
		return false;
	if (!readValue(ifd))
		return false;

	return true;
}
