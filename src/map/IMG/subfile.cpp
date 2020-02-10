#include <QFile>
#include "img.h"
#include "subfile.h"


bool SubFile::seek(Handle &handle, quint32 pos) const
{
	if (handle._file) {
		int blockNum = pos / BLOCK_SIZE;

		if (handle._blockNum != blockNum) {
			if (!handle._file->seek((qint64)blockNum * BLOCK_SIZE))
				return false;
			if (handle._file->read(handle._data.data(), BLOCK_SIZE) < 0)
				return false;
			handle._blockNum = blockNum;
		}

		handle._blockPos = pos % BLOCK_SIZE;
		handle._pos = pos;

		return true;
	} else {
		quint32 blockSize = _img->blockSize();
		int blockNum = pos / blockSize;

		if (handle._blockNum != blockNum) {
			if (blockNum >= _blocks->size())
				return false;
			if (!_img->readBlock(_blocks->at(blockNum), handle._data.data()))
				return false;
			handle._blockNum = blockNum;
		}

		handle._blockPos = pos % blockSize;
		handle._pos = pos;

		return true;
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

bool SubFile::readVBitfield32(Handle &hdl, quint32 &bitfield) const
{
	quint8 bits;

	if (!readUInt8(hdl, bits))
		return false;

	if (!(bits & 1)) {
		seek(hdl, hdl._pos - 1);
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
