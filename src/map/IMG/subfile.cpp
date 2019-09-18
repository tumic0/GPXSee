#include <QFile>
#include "img.h"
#include "subfile.h"

SubFile::Type SubFile::type(const char str[3])
{
	if (!memcmp(str, "TRE", 3))
		return TRE;
	else if (!memcmp(str, "RGN", 3))
		return RGN;
	else if (!memcmp(str, "LBL", 3))
		return LBL;
	else if (!memcmp(str, "TYP", 3))
		return TYP;
	else if (!memcmp(str, "GMP", 3))
		return GMP;
	else if (!memcmp(str, "NET", 3))
		return NET;
	else
		return Unknown;
}

SubFile::SubFile(QFile *file) :_gmpOffset(0), _img(0), _file(file), _blocks(0)
{
	if (!_file->open(QIODevice::ReadOnly))
		qWarning("Error opening %s: %s", qPrintable(_file->fileName()),
		  qPrintable(_file->errorString()));
}

bool SubFile::seek(Handle &handle, quint32 pos) const
{
	Q_ASSERT(_img || _file);

	if (_file)
		return _file->seek(pos);
	else {
		quint32 blockSize = _img->blockSize();
		int blockNum = pos / blockSize;

		if (handle.blockNum != blockNum) {
			if (blockNum >= _blocks->size())
				return false;
			if (!_img->readBlock(_blocks->at(blockNum), handle.data))
				return false;
			handle.blockNum = blockNum;
		}

		handle.blockPos = pos % blockSize;
		handle.pos = pos;

		return true;
	}
}

bool SubFile::readByte(Handle &handle, quint8 &val) const
{
	Q_ASSERT(_img || _file);

	if (_file)
		return _file->getChar((char*)&val);
	else {
		val = handle.data.at(handle.blockPos++);
		handle.pos++;
		return (handle.blockPos >= _img->blockSize())
		  ? seek(handle, handle.pos) : true;
	}
}

bool SubFile::readVUInt32(Handle &hdl, quint32 &val) const
{
	quint8 bytes, shift, b;

	if (!readByte(hdl, b))
		return false;

	if ((b & 1) == 0) {
		if ((b & 2) == 0) {
			bytes = ((b >> 2) & 1) ^ 3;
			shift = 5;
		} else {
			shift = 6;
			bytes = 1;
		}
	} else {
		shift = 7;
		bytes = 0;
	}

	val = b >> (8 - shift);

	for (int i = 1; i <= bytes; i++) {
		if (!readByte(hdl, b))
			return false;
		val |= (((quint32)b) << (i * 8)) >> (8 - shift);

	}

	return true;
}

QString SubFile::fileName() const
{
	return _file ? _file->fileName() : _img->fileName();
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const SubFile &file)
{
	bool continuous = true;
	for (int i = 1; i < file._blocks->size(); i++) {
		if (file._blocks->at(i) != file._blocks->at(i-1) + 1) {
			continuous = false;
			break;
		}
	}

	dbg.nospace() << "SubFile(" << file._blocks->size() << ", "
	  << continuous << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
