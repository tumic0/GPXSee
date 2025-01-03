#ifndef IMG_NODFILE_H
#define IMG_NODFILE_H

#include "section.h"
#include "subfile.h"

namespace IMG {

class NODFile : public SubFile
{
public:
	struct BlockInfo
	{
		struct BlockHeader
		{
			quint32 nodeLonBase;
			quint32 nodeLatBase;
			quint32 linkInfoOffsetBase;
			quint16 flags;
			quint16 linkInfoSize;
			quint8 linksCount;
			quint8 nodesCount;
			quint8 linkTypesCount;

			quint32 size() const {return 0x13 + ((flags >> 0xb) & 1);}
		};

		quint32 offset;
		BlockHeader hdr;
	};

	struct LinkInfo
	{
		quint32 linkOffset;
		quint32 nodeOffset;
		quint32 flags;
	};

	struct NodeInfo
	{
		QPoint pos;
		quint8 flags;
	};

	struct AdjacencyInfo
	{
		AdjacencyInfo(Handle &hdl, const BlockInfo &blockInfo,
		  quint32 linkId, const LinkInfo &linkInfo) : extHdl(hdl),
		  blockInfo(blockInfo), nodeOffset(linkInfo.nodeOffset),
		  linkOffset(linkInfo.linkOffset), linkId(linkId)
		{}

		Handle &extHdl;
		BlockInfo blockInfo;
		NodeInfo nodeInfo;
		quint32 nodeOffset;
		quint32 linkOffset;
		quint32 linkId;
		bool eog;
	};

	NODFile(const IMGData *img)
	  : SubFile(img), _indexFlags(0), _indexRecordSize(0), _blockRecordSize(0),
	  _blockShift(0), _nodeShift(0), _indexIdSize(0) {}
	NODFile(const QString &path)
	  : SubFile(path), _indexFlags(0), _indexRecordSize(0), _blockRecordSize(0),
	  _blockShift(0), _nodeShift(0), _indexIdSize(0) {}
	NODFile(const SubFile *gmp, quint32 offset)
	  : SubFile(gmp, offset), _indexFlags(0), _indexRecordSize(0),
	  _blockRecordSize(0), _blockShift(0), _nodeShift(0), _indexIdSize(0) {}

	bool load(Handle &hdl);

	quint32 indexIdSize() const {return _indexIdSize;}
	bool blockInfo(Handle &hdl, quint32 blockId, BlockInfo &blockInfo) const;
	bool linkInfo(Handle &hdl, const BlockInfo &blockInfo, quint32 linkId,
	  LinkInfo &linkInfo) const;
	bool linkType(Handle &hdl, const BlockInfo &blockInfo, quint8 linkId,
	  quint32 &type) const;
	int nextNode(Handle &hdl, AdjacencyInfo &adj) const;

private:
	bool nodeOffset(Handle &hdl, const BlockInfo &blockInfo, quint8 nodeId,
	  quint32 &nodeOffset) const;
	bool nodeBlock(Handle &hdl, quint32 nodeOffset, BlockInfo &blockInfo) const;
	bool readBlock(Handle &hdl, quint32 blockOffset, BlockInfo &blockInfo) const;
	bool nodeInfo(Handle &hdl, AdjacencyInfo &adj) const;
	bool absAdjInfo(Handle &hdl, AdjacencyInfo &adj) const;
	bool relAdjInfo(Handle &hdl, AdjacencyInfo &adj) const;
	bool adjacencyInfo(Handle &hdl, AdjacencyInfo &adj) const
	{
		return (adj.nodeInfo.flags & 0x20) ? absAdjInfo(hdl, adj)
		  : relAdjInfo(hdl, adj);
	}

	Section _block, _index;
	quint32 _flags, _indexFlags;
	quint16 _indexRecordSize, _blockRecordSize;
	quint8 _blockShift, _nodeShift, _indexIdSize;
};

}

#endif // IMG_NETFILE_H
