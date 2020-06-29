#include "bitstream.h"
#include "nodfile.h"

bool NODFile::init(Handle &hdl)
{
	quint16 hdrLen;

	if (!(seek(hdl, _gmpOffset) && readUInt16(hdl, hdrLen)))
		return false;

	if (hdrLen >= 0x7b) {
		if (!(seek(hdl, _gmpOffset + 0x21) && readUInt8(hdl, _blockShift)
		  && readUInt8(hdl, _nodeShift)))
			return false;
		if (!(seek(hdl, _gmpOffset + 0x67) && readUInt32(hdl, _blockOffset)
		  && readUInt32(hdl, _blockSize) && readUInt16(hdl, _blockRecordSize)
		  && readUInt32(hdl, _indexOffset) && readUInt32(hdl, _indexSize)
		  && readUInt16(hdl, _indexRecordSize) && readUInt32(hdl, _indexFlags)))
			return false;
	}

	return true;
}

quint32 NODFile::indexIdSize(Handle &hdl)
{
	if (!_indexRecordSize && !init(hdl))
		return 0;

	quint32 indexCount = _indexSize / _indexRecordSize;
	if (indexCount <= 0x100)
		return 1;
	else if (indexCount <= 0x1000)
		return 2;
	else if (indexCount <= 0x1000000)
		return 3;
	else
		return 0;
}

bool NODFile::blockInfo(Handle &hdl, quint32 blockIndexId,
  BlockInfo &blockInfo) const
{
	quint32 blockOffset;
	quint32 offset = _indexRecordSize * blockIndexId + _indexOffset;
	quint32 offsetSize = (_indexFlags & 3) + 1;


	Q_ASSERT(offset <= _indexOffset + _indexSize);
	if (!(seek(hdl, offset) && readVUInt32(hdl, offsetSize, blockOffset)))
		return false;

	blockInfo.offset = (blockOffset << _blockShift) + _blockOffset;

	if (!(seek(hdl, blockInfo.offset) && readUInt16(hdl, blockInfo.h0)
	  && readUInt32(hdl, blockInfo.h2) && readUInt32(hdl, blockInfo.h6)
	  && readUInt32(hdl, blockInfo.ha) && readUInt16(hdl, blockInfo.he)
	  && readUInt8(hdl, blockInfo.h10) && readUInt8(hdl, blockInfo.h11)
	  && readUInt8(hdl, blockInfo.h12)))
		return false;

	return true;
}

bool NODFile::linkInfo(Handle &hdl, const BlockInfo &blockInfo, quint32 linkId,
  LinkInfo &linkInfo) const
{
	Q_ASSERT(linkId < blockInfo.h10);

	quint32 infoOffset = ((blockInfo.he * linkId) >> 3) + 0x13
	  + ((blockInfo.h0 >> 0xb) & 1) + blockInfo.offset;
	quint32 s1 = ((blockInfo.h0 >> 2) & 0x1f) + 8;
	quint32 s2 = (blockInfo.h0 >> 7) & 0xf;
	quint32 skip = (blockInfo.he * linkId) & 7;

	Q_ASSERT(infoOffset <= _blockOffset + _blockSize);
	if (!seek(hdl, infoOffset))
		return false;

	quint32 unused, flags;
	BitStream1 bs(*this, hdl, _blockOffset + _blockSize - infoOffset);
	if (!(bs.read(skip, unused) && bs.read(0xc, flags)))
		return false;

	linkInfo.flags = ((flags << 8) & 0xf0000) | (flags & 0xff);

	if (!(flags << 8 & 0x10000)) {
		if (!bs.read(s1, linkInfo.linkOffset))
			return false;
	} else {
		if (!bs.read(s1 - s2, linkInfo.linkOffset))
			return false;
		linkInfo.linkOffset += blockInfo.ha;
	}

	return true;
}

bool NODFile::linkType(Handle &hdl, const BlockInfo &blockInfo, quint8 linkId,
  quint32 &type) const
{
	quint32 offset = ((blockInfo.h10 * blockInfo.he + 7) >> 3) + 0x13 +
	  blockInfo.offset + ((blockInfo.h0 >> 0xb) & 1) + (quint32)blockInfo.h11
	  * 3;
	quint32 low = 0;
	quint32 high = blockInfo.h12 - 1;
	quint32 pos;
	quint16 val;

	if (high > 1) {
		do {
			pos = (low + high) / 2;

			if (!seek(hdl, offset + _blockRecordSize * pos))
				return false;
			if (!readUInt16(hdl, val))
				return false;

			quint32 tmp = pos;
			if ((val >> 8) <= linkId) {
				low = pos;
				tmp = high;
			}
			high = tmp;
		} while (low + 1 < high);
	}

	if (!seek(hdl, offset + _blockRecordSize * low))
		return false;
	if (!readUInt16(hdl, val))
		return false;

	type = val & 0x3f;

	if ((low < high) && (pos != high)) {
		if (!seek(hdl, offset + _blockRecordSize * high))
			return false;
		if (!readUInt16(hdl, val))
			return false;
		if ((val >> 8) <= linkId) {
			type = (val & 0x3f);
		}
	}

	type *= 256;

	return true;
}
