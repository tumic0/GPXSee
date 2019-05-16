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

SubFile::SubFile(QFile *file) : _img(0), _file(file), _size(0)
{
	if (!_file->open(QIODevice::ReadOnly))
		qWarning("Error opening %s: %s", qPrintable(_file->fileName()),
		  qPrintable(_file->errorString()));
}

bool SubFile::isValid() const
{
	return _file
	  ? _file->isOpen()
	  : ((quint32)_img->blockSize() * (quint32)_blocks.size() - _size
		  < (quint32)_img->blockSize());
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
			if (blockNum >= _blocks.size())
				return false;
			if (!_img->readBlock(_blocks.at(blockNum), handle.data))
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

quint32 SubFile::size() const
{
	return _img ? _size : (quint32)_file->size();
}

QString SubFile::fileName() const
{
	return _img ? _img->fileName() : _file->fileName();
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const SubFile &file)
{
	bool continuous = true;
	for (int i = 1; i < file._blocks.size(); i++) {
		if (file._blocks.at(i) != file._blocks.at(i-1) + 1) {
			continuous = false;
			break;
		}
	}

	dbg.nospace() << "SubFile(" << file._size << ", " << file._blocks.size()
	  << ", " << continuous << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
