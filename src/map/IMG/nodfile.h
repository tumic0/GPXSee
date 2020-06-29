#ifndef NODFILE_H
#define NODFILE_H

#include "img.h"
#include "subfile.h"

class NODFile : public SubFile
{
public:
	struct BlockInfo {
		quint32 offset;
		quint16 h0;
		quint32 h2;
		quint32 h6;
		quint32 ha;
		quint16 he;
		quint8 h10; // links count
		quint8 h11;
		quint8 h12;
	};

	struct LinkInfo {
		quint32 linkOffset;
		quint32 flags;
	};

	NODFile(IMG *img) : SubFile(img), _indexOffset(0), _indexSize(0),
	  _indexFlags(0), _blockOffset(0), _blockSize(0), _indexRecordSize(0),
	  _blockRecordSize(0), _blockShift(0), _nodeShift(0) {}
	NODFile(const QString &path) : SubFile(path), _indexOffset(0), _indexSize(0),
	  _indexFlags(0), _blockOffset(0), _blockSize(0), _indexRecordSize(0),
	  _blockRecordSize(0), _blockShift(0), _nodeShift(0) {}
	NODFile(SubFile *gmp, quint32 offset) : SubFile(gmp, offset),
	  _indexOffset(0), _indexSize(0),_indexFlags(0), _blockOffset(0),
	  _blockSize(0), _indexRecordSize(0), _blockRecordSize(0), _blockShift(0),
	  _nodeShift(0) {}

	quint32 indexIdSize(Handle &hdl);
	bool blockInfo(Handle &hdl, quint32 blockIndexId,
	  BlockInfo &blockInfo) const;
	bool linkInfo(Handle &hdl, const BlockInfo &blockInfo, quint32 linkId,
	  LinkInfo &linkInfo) const;
	bool linkType(Handle &hdl, const BlockInfo &blockInfo, quint8 linkId,
	  quint32 &type) const;

private:
	bool init(Handle &hdl);

	quint32 _indexOffset, _indexSize, _indexFlags, _blockOffset, _blockSize;
	quint16 _indexRecordSize, _blockRecordSize;
	quint8 _blockShift, _nodeShift;
};

#endif // NETFILE_H
