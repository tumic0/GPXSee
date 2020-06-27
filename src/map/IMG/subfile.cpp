#include <QFile>
#include "img.h"
#include "subfile.h"


#define mod2n(x, m) ((x) & ((m) - 1));

bool SubFile::seek(Handle &handle, quint32 pos) const
{
	if (handle._file) {
		int blockNum = pos >> BLOCK_BITS;

		if (handle._blockNum != blockNum) {
			if (!handle._file->seek((quint64)blockNum << BLOCK_BITS))
				return false;
			if (handle._file->read(handle._data.data(), (1<<BLOCK_BITS)) < 0)
				return false;
			handle._blockNum = blockNum;
		}

		handle._blockPos = mod2n(pos, 1U<<BLOCK_BITS);
		handle._pos = pos;

		return true;
	} else {
		quint32 blockBits = _img->blockBits();
		int blockNum = pos >> blockBits;

		if (handle._blockNum != blockNum) {
			if (blockNum >= _blocks->size())
				return false;
			if (!_img->readBlock(_blocks->at(blockNum), handle._data.data()))
				return false;
			handle._blockNum = blockNum;
		}

		handle._blockPos = mod2n(pos, 1U<<blockBits);
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

bool SubFile::readVUInt32(Handle &hdl, quint32 bytes, quint32 &val) const
{
	switch (bytes) {
		case 1:
			return readUInt8(hdl, val);
		case 2:
			return readUInt16(hdl, val);
		case 3:
			return readUInt24(hdl, val);
		case 4:
			return readUInt32(hdl, val);
		default:
			return false;
	}
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
