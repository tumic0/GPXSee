#include "bitstream.h"
#include "nodfile.h"


#define ARRAY_SIZE(array) \
  (sizeof(array) / sizeof(array[0]))

static const struct
{
	quint8 lon;
	quint8 lat;
} LLBITS[] = {
	{0xc, 0xc}, {0x8, 0x10}, {0x10, 0x8}, {0x10, 0x10}, {0xc, 0x14},
	{0x14, 0xc}, {0x14, 0x14}
};

struct NodeOffset
{
	bool ext;
	union {
		qint32 offset;
		quint8 id;
	} u;
};

static bool adjDistInfo(BitStream1 &bs, bool extraBit, bool &eog)
{
	quint32 data, cnt;

	if (!bs.read(extraBit | 8, data))
		return false;

	data <<= !extraBit;
	eog |= (quint8)data & 1;
	data >>= 1;

	for (cnt = 0; (data >> cnt) & 1; cnt++) {
		if (cnt == 4)
			break;
	}
	if (!bs.read(cnt * 4, data))
		return false;

	return true;
}

static bool adjNodeInfo(BitStream1 &bs, bool extraBit, NodeOffset &offset)
{
	quint32 data;

	if (!bs.read(9, data))
		return false;

	data <<= !extraBit;

	if (data & 1) {
		offset.ext = true;
		offset.u.id = data >> 1;
	} else {
		quint32 bits = (data >> 1) & 7;
		quint32 data2;

		if (!bs.read(bits + extraBit + 1, data2))
			return false;

		data = data2 << (6 - extraBit) | data >> 4;
		bits = 0x19 - bits;
		offset.ext = false;
		offset.u.offset = ((qint32)(data << bits) >> bits);
	}

	return true;
}

static bool skipOptAdjData(BitStream1 &bs)
{
	// TODO
	Q_UNUSED(bs);
	Q_ASSERT(false);
	return false;
}


bool NODFile::init(Handle &hdl)
{
	quint16 hdrLen;

	if (!(seek(hdl, _gmpOffset) && readUInt16(hdl, hdrLen)))
		return false;
	if (hdrLen < 0x7b)
		return false;

	if (!(seek(hdl, _gmpOffset + 0x1d) && readUInt32(hdl, _flags)
	  && readUInt8(hdl, _blockShift) && readUInt8(hdl, _nodeShift)))
		return false;

	if (!(seek(hdl, _gmpOffset + 0x67) && readUInt32(hdl, _blockOffset)
	  && readUInt32(hdl, _blockSize) && readUInt16(hdl, _blockRecordSize)
	  && readUInt32(hdl, _indexOffset) && readUInt32(hdl, _indexSize)
	  && readUInt16(hdl, _indexRecordSize) && readUInt32(hdl, _indexFlags)))
		return false;

	if (!_indexRecordSize)
		return false;
	quint32 indexCount = _indexSize / _indexRecordSize;
	if (indexCount <= 0x100)
		_indexIdSize = 1;
	else if (indexCount <= 0x1000)
		_indexIdSize = 2;
	else if (indexCount <= 0x1000000)
		_indexIdSize = 3;

	return (_indexIdSize > 0);
}

quint32 NODFile::indexIdSize(Handle &hdl)
{
	if (!_indexIdSize && !init(hdl))
		return 0;

	return _indexIdSize;
}

bool NODFile::readBlock(Handle &hdl, quint32 blockOffset,
  BlockInfo &blockInfo) const
{
	blockInfo.offset = blockOffset;

	if (!(seek(hdl, blockInfo.offset + _blockOffset)
	  && readUInt16(hdl, blockInfo.hdr.s0) && readUInt32(hdl, blockInfo.hdr.s2)
	  && readUInt32(hdl, blockInfo.hdr.s6) && readUInt32(hdl, blockInfo.hdr.sa)
	  && readUInt16(hdl, blockInfo.hdr.se) && readUInt8(hdl, blockInfo.hdr.s10)
	  && readUInt8(hdl, blockInfo.hdr.s11) && readUInt8(hdl, blockInfo.hdr.s12)))
		return false;

	return true;
}

bool NODFile::blockInfo(Handle &hdl, quint32 blockId, BlockInfo &blockInfo) const
{
	quint32 blockOffset;
	quint32 offset = _indexRecordSize * blockId + _indexOffset;
	quint32 offsetSize = (_indexFlags & 3) + 1;

	if (offset > _indexOffset + _indexSize)
		return false;
	if (!(seek(hdl, offset) && readVUInt32(hdl, offsetSize, blockOffset)))
		return false;

	return readBlock(hdl, blockOffset << _blockShift, blockInfo);
}

bool NODFile::linkInfo(Handle &hdl, const BlockInfo &blockInfo, quint32 linkId,
  LinkInfo &linkInfo) const
{
	if (linkId >= blockInfo.hdr.s10)
		return false;

	quint32 infoOffset = ((blockInfo.hdr.se * linkId) >> 3) + 0x13
	  + ((blockInfo.hdr.s0 >> 0xb) & 1) + blockInfo.offset + _blockOffset;
	quint32 s1 = ((blockInfo.hdr.s0 >> 2) & 0x1f) + 8;
	quint32 s2 = (blockInfo.hdr.s0 >> 7) & 0xf;
	quint32 skip = (blockInfo.hdr.se * linkId) & 7;

	if (infoOffset > _blockOffset + _blockSize || infoOffset < blockInfo.offset)
		return false;
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
		linkInfo.linkOffset += blockInfo.hdr.sa;
	}

	if (!bs.read(s2, linkInfo.nodeOffset))
		return false;
	linkInfo.nodeOffset = (blockInfo.offset - linkInfo.nodeOffset)
	  >> _nodeShift;

	return true;
}

bool NODFile::nodeInfo(Handle &hdl, const BlockInfo &blockInfo,
  quint32 nodeOffset, NodeInfo &nodeInfo) const
{
	quint32 infoOffset = (nodeOffset << _nodeShift) + _blockOffset;
	if (infoOffset > _blockOffset + _blockSize || infoOffset < blockInfo.offset)
		return false;
	if (!seek(hdl, infoOffset))
		return false;

	BitStream1 bs(*this, hdl, _blockOffset + _blockSize - infoOffset);

	if (!bs.read(8, nodeInfo.flags))
		return false;

	if ((nodeInfo.flags & 7) >= ARRAY_SIZE(LLBITS))
		return false;
	quint8 lonBits = LLBITS[nodeInfo.flags & 7].lon;
	quint8 latBits = LLBITS[nodeInfo.flags & 7].lat;
	quint8 maxBits = ((_flags >> 10) & 7) | 0x18;

	quint32 lon, lat;
	if (!(bs.read(lonBits, lon) && bs.read(latBits, lat)))
		return false;

	quint8 lonShift = 0x20 - lonBits;
	quint8 latShift = 0x20 - latBits;
	quint8 shift = 0x20 - maxBits;
	QPoint pos((((int)(lon << lonShift) >> lonShift) << shift)
	  + blockInfo.hdr.s2, (((int)(lat << latShift) >> latShift) << shift)
	  + blockInfo.hdr.s6);
	nodeInfo.bytes = ((lonBits + latBits) >> 3) + 1;

	if ((maxBits < 0x1c) && (nodeInfo.flags & 8)) {
		quint8 extraBits = 0x1c - maxBits;
		quint32 extraLon, extraLat;

		if (!(bs.read(extraBits, extraLon) && bs.read(extraBits, extraLat)))
			return false;
		pos.setX(pos.x() | extraLon << 4); pos.setY(pos.y() | extraLat << 4);
		nodeInfo.bytes++;
	}
	// TODO?: extra bits

	nodeInfo.pos = pos;
	nodeInfo.flags &= 0xf8;

	return true;
}

bool NODFile::nodeOffset(Handle &hdl, const BlockInfo &blockInfo,
  quint8 nodeId, quint32 &nodeOffset) const
{
	if (nodeId >= blockInfo.hdr.s11)
		return false;

	quint32 offset = ((blockInfo.hdr.s10 * blockInfo.hdr.se + 7) >> 3)
	  + 0x13 + nodeId * 3 + _blockOffset + blockInfo.offset
	  + ((blockInfo.hdr.s0 >> 0xb) & 1);

	if (!(seek(hdl, offset) && readUInt24(hdl, nodeOffset)))
		return false;

	return true;
}

bool NODFile::nodeBlock(Handle &hdl, quint32 nodeOffset,
  BlockInfo &blockInfo) const
{
	int low = 0;
	int high = _indexSize / _indexRecordSize - 1;
	quint32 offsetSize = (_indexFlags & 3) + 1;

	while (low <= high) {
		quint32 m = ((low + high) / 2);
		quint32 offset = _indexRecordSize * m + _indexOffset;
		quint32 blockOffset, prevBlockOffset;

		if (m > 0) {
			if (!(seek(hdl, offset - _indexRecordSize)
			  && readVUInt32(hdl, offsetSize, prevBlockOffset)
			  && readVUInt32(hdl, offsetSize, blockOffset)))
				return false;
		} else {
			if (!(seek(hdl, offset)
			  && readVUInt32(hdl, offsetSize, blockOffset)))
				return false;
			prevBlockOffset = 0;
		}
		prevBlockOffset <<= _blockShift;
		blockOffset <<= _blockShift;

		if (blockOffset < nodeOffset)
			low = m + 1;
		else {
			if (prevBlockOffset <= nodeOffset)
				return readBlock(hdl, blockOffset, blockInfo);
			else
				high = m - 1;
		}
	}

	return false;
}

bool NODFile::absAdjInfo(Handle &hdl, AdjacencyInfo &adj) const
{
	quint32 infoOffset = (adj.nodeOffset << _nodeShift) + _blockOffset
	  + adj.nodeInfo.bytes;
	if (!seek(hdl, infoOffset))
		return false;
	BitStream1 bs(*this, hdl, _blockOffset + _blockSize - infoOffset);

	quint8 linkId = adj.blockInfo.hdr.s10;
	quint32 m2p = 2;
	quint32 skip = 8;
	quint32 flags;
	quint32 nextOffset = 0xFFFFFFFF;
	bool extraBit = (adj.nodeInfo.flags >> 6) & 1;
	bool linkIdValid = true;
	bool firstLoop = true;
	NodeOffset offset;

	do {
		adj.eog = false;

		if (!bs.read(8, flags))
			return false;

		if (firstLoop) {
			skip >>= (flags >> 5) & 1;
			flags |= 0x20;
		}
		firstLoop = false;
		quint32 f4 = flags & 0x10;
		quint32 f4sn = (f4 >> 4) ^ 1;
		quint32 m1 = (flags >> 5) & f4sn;
		quint32 m2 = (f4 >> 3) | (f4sn & (flags >> 6));

		if (m1) {
			if (!bs.read(8, linkId))
				return false;
			linkIdValid = true;
		}

		if ((m2 != m2p) || (flags & 0x10) || m1) {
			quint32 data;
			if (!bs.read(skip, data))
				return false;
		}

		if (!(flags & 0x10)) {
			if (!adjDistInfo(bs, (m2 == 1 && linkIdValid), adj.eog))
				return false;

			if (!adjNodeInfo(bs, extraBit, offset))
				return false;
			if (!offset.ext)
				nextOffset = adj.nodeOffset + offset.u.offset;
			else if (!nodeOffset(adj.extHdl, adj.blockInfo, offset.u.id,
			  nextOffset))
				return false;

			m2p = m2;
		}

		if (flags & 0x8) {
			quint32 data;
			if (!bs.read(8, data))
				return false;
			if (!(data & 0xe0)) {
				if (!bs.read(8, data))
					return false;
			}
		}

		if ((_flags & 0x18) && !skipOptAdjData(bs))
			return false;

		if ((m2 == 1) && linkIdValid) {
			LinkInfo li;
			if (adj.linkId == 0xFFFFFFFF) {
				if (!linkInfo(adj.extHdl, adj.blockInfo, linkId, li))
					return false;
			} else
				li.linkOffset = 0xFFFFFFFF;

			if ((adj.linkOffset == li.linkOffset) || (adj.linkId == linkId)) {
				adj.nodeOffset = nextOffset;
				if (offset.ext) {
					adj.linkId = 0xFFFFFFFF;
					return nodeBlock(hdl, adj.nodeOffset << _nodeShift,
					  adj.blockInfo);
				} else {
					adj.linkId = linkId;
					return true;
				}
			}

			linkIdValid = false;
		}
	} while (!(flags & 0x80));

	adj.nodeOffset = 0xFFFFFFFF;

	return true;
}

bool NODFile::relAdjInfo(Handle &hdl, AdjacencyInfo &adj) const
{
	quint32 infoOffset = (adj.nodeOffset << _nodeShift) + _blockOffset
	  + adj.nodeInfo.bytes;
	if (!seek(hdl, infoOffset))
		return false;

	BitStream1 bs(*this, hdl, _blockOffset + _blockSize - infoOffset);

	quint32 linkId = adj.blockInfo.hdr.s10;
	quint32 skip = 8;
	quint32 flagsBits = 8;
	quint32 flags;
	quint32 nextOffset = 0xFFFFFFFF;
	NodeOffset offset;
	bool extraBit = (adj.nodeInfo.flags >> 6) & 1;
	bool linkIdValid = true;
	bool firstLoop = true;

	do {
		adj.eog = false;

		if (!bs.read(flagsBits, flags))
			return false;

		flags <<= (8U - flagsBits);
		if (firstLoop) {
			skip >>= (flags >> 5) & 1;
			flags = ((flags >> 1) & 0x20) | (flags & 0xffffffdf);
		}
		firstLoop = false;
		flagsBits >>= (flags >> 3) & 1;
		quint32 m = (((flags & 0x70) == 0x30) << 1) | ((flags >> 6) & 1);
		if (!m) {
			adj.nodeOffset = 0xFFFFFFFF;
			return true;
		}

		if ((flags & 0x60) == 0x60) {
			if (!bs.read(8, linkId))
				return false;
			linkIdValid = true;
		}

		if (((flags & 0x70) == 0x70)) {
			quint32 data;
			if (!bs.read(skip, data))
				return false;
		}

		if ((flags & 0x50) == 0x50) {
			if (!adjDistInfo(bs, false, adj.eog))
				return false;
			adj.eog = true;
		}
		if ((flags >> 6) & 1) {
			if (!adjNodeInfo(bs, extraBit, offset))
				return false;
			if (!offset.ext)
				nextOffset = adj.nodeOffset + offset.u.offset;
			else if (!nodeOffset(adj.extHdl, adj.blockInfo, offset.u.id,
			  nextOffset))
				return false;
		}

		if ((_flags & 0x18) && !skipOptAdjData(bs))
			return false;

		if (((m == 1) && linkIdValid)) {
			LinkInfo li;
			if (adj.linkId == 0xFFFFFFFF) {
				if (!linkInfo(adj.extHdl, adj.blockInfo, linkId, li))
					return false;
			} else
				li.linkOffset = 0xFFFFFFFF;

			if ((adj.linkOffset == li.linkOffset) || (adj.linkId == linkId)) {
				adj.nodeOffset = nextOffset;
				if (offset.ext) {
					adj.linkId = 0xFFFFFFFF;
					return nodeBlock(hdl, adj.nodeOffset << _nodeShift,
					  adj.blockInfo);
				} else {
					adj.linkId = linkId;
					return true;
				}
			}
			linkIdValid = false;
		}
	} while (!(flags & 0x80));

	adj.nodeOffset = 0xFFFFFFFF;

	return true;
}

int NODFile::nextNode(Handle &hdl, AdjacencyInfo &adjInfo)
{
	if (adjInfo.nodeOffset == 0xFFFFFFFF)
		return 1;

	if (!nodeInfo(hdl, adjInfo.blockInfo, adjInfo.nodeOffset,
	  adjInfo.nodeInfo))
		return -1;
	if (!adjacencyInfo(hdl, adjInfo))
		return -1;

	return 0;
}

bool NODFile::linkType(Handle &hdl, const BlockInfo &blockInfo, quint8 linkId,
  quint32 &type) const
{
	quint32 offset = ((blockInfo.hdr.s10 * blockInfo.hdr.se + 7) >> 3) + 0x13
	  + blockInfo.offset + _blockOffset + ((blockInfo.hdr.s0 >> 0xb) & 1)
	  + blockInfo.hdr.s11 * 3;
	quint32 low = 0;
	quint32 high = blockInfo.hdr.s12 - 1;
	quint32 pos = blockInfo.hdr.s12;
	quint16 val;

	if (high > 1) {
		do {
			pos = (low + high) / 2;

			if (!(seek(hdl, offset + _blockRecordSize * pos)
			  && readUInt16(hdl, val)))
				return false;

			if ((val >> 8) <= linkId)
				low = pos;
			else
				high = pos;
		} while (low + 1 < high);
	}

	if (!(seek(hdl, offset + _blockRecordSize * low) && readUInt16(hdl, val)))
		return false;

	type = val & 0x3f;

	if ((low < high) && (pos != high)) {
		if (!(seek(hdl, offset + _blockRecordSize * high)
		  && readUInt16(hdl, val)))
			return false;
		if ((val >> 8) <= linkId)
			type = (val & 0x3f);
	}

	type <<= 8;

	return true;
}
