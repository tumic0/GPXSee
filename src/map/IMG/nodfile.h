#ifndef NODFILE_H
#define NODFILE_H

#include "img.h"
#include "subfile.h"

class NODFile : public SubFile
{
public:
	struct BlockInfo
	{
		quint32 offset;
		struct
		{
			quint32 s2; // node lon base
			quint32 s6; // node lat base
			quint32 sa;
			quint16 s0; // flags
			quint16 se; // link info bit size
			quint8 s10; // links count
			quint8 s11; // nodes count
			quint8 s12; // link types count
		} hdr;
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
		quint8 bytes;
	};

	struct AdjacencyInfo
	{
		AdjacencyInfo(const SubFile *file, const BlockInfo &blockInfo,
		  quint32 linkId, const LinkInfo &linkInfo) : extHdl(file),
		  blockInfo(blockInfo), nodeOffset(linkInfo.nodeOffset),
		  linkOffset(linkInfo.linkOffset), linkId(linkId)
		{}

		Handle extHdl;
		BlockInfo blockInfo;
		NodeInfo nodeInfo;
		quint32 nodeOffset;
		quint32 linkOffset;
		quint32 linkId;
		quint8 flags;
	};

	NODFile(IMG *img) : SubFile(img), _indexOffset(0), _indexSize(0),
	  _indexFlags(0), _blockOffset(0), _blockSize(0), _indexRecordSize(0),
	  _blockRecordSize(0), _blockShift(0), _nodeShift(0), _indexIdSize(0) {}
	NODFile(const QString &path) : SubFile(path), _indexOffset(0), _indexSize(0),
	  _indexFlags(0), _blockOffset(0), _blockSize(0), _indexRecordSize(0),
	  _blockRecordSize(0), _blockShift(0), _nodeShift(0), _indexIdSize(0) {}
	NODFile(SubFile *gmp, quint32 offset) : SubFile(gmp, offset),
	  _indexOffset(0), _indexSize(0), _indexFlags(0), _blockOffset(0),
	  _blockSize(0), _indexRecordSize(0), _blockRecordSize(0), _blockShift(0),
	  _nodeShift(0), _indexIdSize(0) {}

	quint32 indexIdSize(Handle &hdl);
	bool blockInfo(Handle &hdl, quint32 blockId, BlockInfo &blockInfo) const;
	bool linkInfo(Handle &hdl, const BlockInfo &blockInfo, quint32 linkId,
	  LinkInfo &linkInfo) const;
	bool linkType(Handle &hdl, const BlockInfo &blockInfo, quint8 linkId,
	  quint32 &type) const;
	int nextNode(Handle &hdl, AdjacencyInfo &adjInfo);

private:
	bool init(Handle &hdl);
	bool nodeInfo(Handle &hdl, const BlockInfo &blockInfo, quint32 nodeOffset,
	  NodeInfo &nodeInfo) const;
	bool nodeOffset(Handle &hdl, const BlockInfo &blockInfo, quint8 nodeId,
	  quint32 &nodeOffset) const;
	bool absAdjInfo(Handle &hdl, AdjacencyInfo &adj) const;
	bool relAdjInfo(Handle &hdl, AdjacencyInfo &adj) const;
	bool adjacencyInfo(Handle &hdl, AdjacencyInfo &adj) const
	{
		return (adj.nodeInfo.flags & 0x20) ? absAdjInfo(hdl, adj)
		  : relAdjInfo(hdl, adj);
	}
	bool nodeBlock(Handle &hdl, quint32 nodeOffset, BlockInfo &blockInfo) const;
	bool readBlock(Handle &hdl, quint32 blockOffset, BlockInfo &blockInfo) const;

	quint32 _flags, _indexOffset, _indexSize, _indexFlags, _blockOffset,
	  _blockSize;
	quint16 _indexRecordSize, _blockRecordSize;
	quint8 _blockShift, _nodeShift, _indexIdSize;
};

#endif // NETFILE_H
