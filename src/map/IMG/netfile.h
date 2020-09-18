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
	  _linksSize(0), _shift(0), _linksShift(0), _init(false) {}
	NETFile(const QString &path) : SubFile(path), _offset(0), _size(0),
	  _linksOffset(0), _linksSize(0), _shift(0), _linksShift(0),
	  _init(false) {}
	NETFile(SubFile *gmp, quint32 offset) : SubFile(gmp, offset),
	  _offset(0), _size(0), _linksOffset(0), _linksSize(0), _shift(0),
	  _linksShift(0), _init(false) {}

	bool lblOffset(Handle &hdl, quint32 netOffset, quint32 &lblOffset);
	bool link(const SubDiv *subdiv, quint32 shift, Handle &hdl, NODFile *nod,
	  Handle &nodHdl, LBLFile *lbl, Handle &lblHdl,
	  const NODFile::BlockInfo blockInfo, quint8 linkId, quint8 lineId,
	  const HuffmanTable &table, QList<IMG::Poly> *lines);

private:
	bool init(Handle &hdl);
	bool linkLabel(Handle &hdl, quint32 offset, quint32 size, LBLFile *lbl,
	  Handle &lblHdl, Label &label);

	quint32 _offset, _size, _linksOffset, _linksSize;
	quint8 _shift, _linksShift;
	quint8 _tableId;
	bool _init;
};

#endif // NETFILE_H
