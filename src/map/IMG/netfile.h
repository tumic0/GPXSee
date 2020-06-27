#ifndef NETFILE_H
#define NETFILE_H

#include "img.h"
#include "subfile.h"
#include "nodfile.h"

class NODFile;
class LBLFile;
class SubDiv;
class HuffmanTable;

class NETFile : public SubFile
{
public:
	NETFile(IMG *img) : SubFile(img), _offset(0), _size(0), _linksOffset(0),
	  _linksSize(0), _multiplier(0), _linksShift(0) {}
	NETFile(const QString &path) : SubFile(path), _offset(0), _size(0),
	  _linksOffset(0), _linksSize(0), _multiplier(0), _linksShift(0) {}
	NETFile(SubFile *gmp, quint32 offset) : SubFile(gmp, offset),
	  _offset(0), _size(0), _linksOffset(0), _linksSize(0), _multiplier(0),
	  _linksShift(0) {}

	bool lblOffset(Handle &hdl, quint32 netOffset, quint32 &lblOffset);
	bool link(const SubDiv *subdiv, Handle &hdl, NODFile *nod, Handle &nodHdl,
	  const NODFile::BlockInfo blockInfo, quint8 linkId, quint8 lineId,
	  const HuffmanTable &table, QList<IMG::Poly> *lines);

private:
	bool init(Handle &hdl);

	quint32 _offset, _size, _linksOffset, _linksSize;
	quint8 _multiplier, _linksShift;
	quint8 _tableId;
};

#endif // NETFILE_H
