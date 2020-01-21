#include <QFile>
#include "img.h"
#include "subfile.h"


bool SubFile::seek(Handle &handle, quint32 pos) const
{
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

bool SubFile::readVUInt32SW(Handle &hdl, quint32 bytes, quint32 &val) const
{
	quint8 b;

	val = 0;
	for (quint32 i = bytes; i; i--) {
		if (!readByte(hdl, b))
			return false;
		val |= ((quint32)b) << ((i-1) * 8);
	}

	return true;
}

bool SubFile::readVBitfield32(Handle &hdl, quint32 &bitfield) const
{
	quint8 bits;

	if (!readUInt8(hdl, bits))
		return false;

	if (!(bits & 1)) {
		seek(hdl, hdl.pos - 1);
		if (!((bits>>1) & 1)) {
			if (!((bits>>2) & 1)) {
				if (!readUInt32(hdl, bitfield))
					return false;
			} else {
				if (!readUInt24(hdl, bitfield))
					return false;
			}
			bitfield >>= 3;
		} else {
			if (!readUInt16(hdl, bitfield))
				return false;
			bitfield >>= 2;
		}
	} else
		bitfield = bits>>1;

	return true;
}

QString SubFile::fileName() const
{
	return _file ? _file->fileName() : _img->fileName();
}
