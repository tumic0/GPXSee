#include "common/util.h"
#include "bitstream.h"
#include "nodfile.h"

using namespace Garmin;
using namespace IMG;

#define DELTA(val, llBits, maxBits) \
  (((int)((val) << (32 - (llBits))) >> (32 - (llBits))) << (32 - (maxBits)))

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

	if (!bs.read((int)extraBit | 8, data))
		return false;
	if (!extraBit)
		data <<= 1;

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
	if (!extraBit)
		data <<= 1;

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


bool NODFile::load(Handle &hdl)
{
	quint16 hdrLen;

	if (!(seek(hdl, _gmpOffset) && readUInt16(hdl, hdrLen)))
		return false;

	if (hdrLen >= 0x7F) {
		if (!(seek(hdl, _gmpOffset + 0x1d) && readUInt32(hdl, _flags)
		  && readByte(hdl, &_blockShift) && readByte(hdl, &_nodeShift)))
			return false;

		if (!(seek(hdl, _gmpOffset + 0x67) && readUInt32(hdl, _block.offset)
		  && readUInt32(hdl, _block.size) && readUInt16(hdl, _blockRecordSize)
		  && readUInt32(hdl, _index.offset) && readUInt32(hdl, _index.size)
		  && readUInt16(hdl, _indexRecordSize) && readUInt32(hdl, _indexFlags)))
			return false;

		if (!_indexRecordSize || _index.size < _indexRecordSize)
			return false;
		quint32 indexCount = _index.size / _indexRecordSize;
		_indexIdSize = byteSize(indexCount - 1);
	}

	return true;
}

bool NODFile::readBlock(Handle &hdl, quint32 blockOffset,
  BlockInfo &blockInfo) const
{
	blockInfo.offset = blockOffset;

	return (seek(hdl, blockInfo.offset + _block.offset)
	  && readUInt16(hdl, blockInfo.hdr.flags)
	  && readUInt32(hdl, blockInfo.hdr.nodeLonBase)
	  && readUInt32(hdl, blockInfo.hdr.nodeLatBase)
	  && readUInt32(hdl, blockInfo.hdr.linkInfoOffsetBase)
	  && readUInt16(hdl, blockInfo.hdr.linkInfoSize)
	  && readByte(hdl, &blockInfo.hdr.linksCount)
	  && readByte(hdl, &blockInfo.hdr.nodesCount)
	  && readByte(hdl, &blockInfo.hdr.linkTypesCount));
}

bool NODFile::blockInfo(Handle &hdl, quint32 blockId, BlockInfo &blockInfo) const
{
	quint32 blockOffset;
	quint32 offset = _indexRecordSize * blockId + _index.offset;
	quint32 offsetSize = (_indexFlags & 3) + 1;

	if (offset > _index.offset + _index.size)
		return false;
	if (!(seek(hdl, offset) && readVUInt32(hdl, offsetSize, blockOffset)))
		return false;

	return readBlock(hdl, blockOffset << _blockShift, blockInfo);
}

bool NODFile::linkInfo(Handle &hdl, const BlockInfo &blockInfo, quint32 linkId,
  LinkInfo &linkInfo) const
{
	if (linkId >= blockInfo.hdr.linksCount)
		return false;

	quint32 infoOffset = _block.offset + blockInfo.offset + blockInfo.hdr.size()
	  + ((blockInfo.hdr.linkInfoSize * linkId) >> 3);
	quint32 s1 = ((blockInfo.hdr.flags >> 2) & 0x1f) + 8;
	quint32 s2 = (blockInfo.hdr.flags >> 7) & 0xf;
	quint32 skip = (blockInfo.hdr.linkInfoSize * linkId) & 7;

	if (infoOffset > _block.offset + _block.size || infoOffset < blockInfo.offset)
		return false;
	if (!seek(hdl, infoOffset))
		return false;

	quint32 padding;
	BitStream1 bs(*this, hdl, _block.offset + _block.size - infoOffset);
	if (!(bs.read(skip, padding) && bs.read(0xc, linkInfo.flags)))
		return false;

	if (!(linkInfo.flags & 0x100)) {
		if (!bs.read(s1, linkInfo.linkOffset))
			return false;
		linkInfo.nodeOffset = 0xFFFFFFFF;
	} else {
		if (!bs.read(s1 - s2, linkInfo.linkOffset))
			return false;
		linkInfo.linkOffset += blockInfo.hdr.linkInfoOffsetBase;

		if (!bs.read(s2, linkInfo.nodeOffset))
			return false;
		linkInfo.nodeOffset = (blockInfo.offset - linkInfo.nodeOffset)
		  >> _nodeShift;
	}

	return true;
}

bool NODFile::nodeInfo(Handle &hdl, AdjacencyInfo &adj) const
{
	quint32 infoOffset = (adj.nodeOffset << _nodeShift) + _block.offset;
	if (infoOffset > _block.offset + _block.size
	  || infoOffset < adj.blockInfo.offset)
		return false;
	if (!seek(hdl, infoOffset))
		return false;

	BitStream1 bs(*this, hdl, _block.offset + _block.size - infoOffset);

	if (!bs.read(8, adj.nodeInfo.flags))
		return false;

	if ((adj.nodeInfo.flags & 7) >= ARRAY_SIZE(LLBITS))
		return false;
	quint8 lonBits = LLBITS[adj.nodeInfo.flags & 7].lon;
	quint8 latBits = LLBITS[adj.nodeInfo.flags & 7].lat;
	quint8 maxBits = ((_flags >> 10) & 7) | 0x18;

	quint32 lon, lat;
	if (!(bs.read(lonBits, lon) && bs.read(latBits, lat)))
		return false;

	QPoint pos(
	  adj.blockInfo.hdr.nodeLonBase + DELTA(lon, lonBits, maxBits),
	  adj.blockInfo.hdr.nodeLatBase + DELTA(lat, latBits, maxBits));

	if ((maxBits < 0x1c) && (adj.nodeInfo.flags & 8)) {
		quint8 extraBits = 0x1c - maxBits;
		quint32 extraLon, extraLat;

		if (!(bs.read(extraBits, extraLon) && bs.read(extraBits, extraLat)))
			return false;
		pos.setX(pos.x() | extraLon << 4); pos.setY(pos.y() | extraLat << 4);
	}
	// TODO?: check and adjust (shift) coordinates

	adj.nodeInfo.pos = pos;

	return true;
}

bool NODFile::nodeOffset(Handle &hdl, const BlockInfo &blockInfo,
  quint8 nodeId, quint32 &nodeOffset) const
{
	if (nodeId >= blockInfo.hdr.nodesCount)
		return false;

	quint32 offset = _block.offset + blockInfo.offset + blockInfo.hdr.size()
	  + bs(blockInfo.hdr.linksCount * blockInfo.hdr.linkInfoSize)
	  + nodeId * 3;

	return (seek(hdl, offset) && readUInt24(hdl, nodeOffset));
}

bool NODFile::nodeBlock(Handle &hdl, quint32 nodeOffset,
  BlockInfo &blockInfo) const
{
	int low = 0;
	int high = _index.size / _indexRecordSize - 1;
	quint32 offsetSize = (_indexFlags & 3) + 1;

	while (low <= high) {
		int m = ((low + high) / 2);
		quint32 offset = _indexRecordSize * m + _index.offset;
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
	BitStream1 bs(*this, hdl, _block.offset + _block.size - pos(hdl));

	quint8 linkId = adj.blockInfo.hdr.linksCount;
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
			firstLoop = false;
		}

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
	BitStream1 bs(*this, hdl, _block.offset + _block.size - pos(hdl));

	quint32 linkId = adj.blockInfo.hdr.linksCount;
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
			firstLoop = false;
		}
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

int NODFile::nextNode(Handle &hdl, AdjacencyInfo &adj) const
{
	if (adj.nodeOffset == 0xFFFFFFFF)
		return 1;

	if (!nodeInfo(hdl, adj))
		return -1;
	if (!adjacencyInfo(hdl, adj))
		return -1;

	return 0;
}

bool NODFile::linkType(Handle &hdl, const BlockInfo &blockInfo, quint8 linkId,
  quint32 &type) const
{
	quint32 offset = _block.offset + blockInfo.offset + blockInfo.hdr.size()
	  + bs(blockInfo.hdr.linksCount * blockInfo.hdr.linkInfoSize)
	  + blockInfo.hdr.nodesCount * 3;
	int low = 0;
	int high = blockInfo.hdr.linkTypesCount - 1;
	quint16 val;

	while (low <= high) {
		int m = ((low + high) / 2);

		if (!(seek(hdl, offset + _blockRecordSize * m) && readUInt16(hdl, val)))
			return false;

		if ((val >> 8) < linkId)
			low = m + 1;
		else if ((val >> 8) > linkId)
			high = m - 1;
		else {
			type = (val & 0x3f) << 8;
			return true;
		}
	}

	if (high < 0)
		return false;
	if ((blockInfo.hdr.linkTypesCount > 1)
	  && !(seek(hdl, offset + _blockRecordSize * high) && readUInt16(hdl, val)))
		return false;
	type = (val & 0x3f) << 8;

	return true;
}
