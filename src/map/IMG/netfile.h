#ifndef IMG_NETFILE_H
#define IMG_NETFILE_H

#include "subfile.h"
#include "nodfile.h"

namespace IMG {

class LBLFile;
class RGNFile;
class SubDiv;
class HuffmanTable;

class NETFile : public SubFile
{
public:
	NETFile(const IMGData *img) : SubFile(img), _huffmanTable(0), _tp(0),
	  _offset(0), _size(0), _linksOffset(0), _linksSize(0), _shift(0),
	  _linksShift(0) {}
	NETFile(const QString *path) : SubFile(path), _huffmanTable(0), _tp(0),
	  _offset(0), _size(0), _linksOffset(0), _linksSize(0), _shift(0),
	  _linksShift(0) {}
	NETFile(const SubFile *gmp, quint32 offset) : SubFile(gmp, offset),
	  _huffmanTable(0), _tp(0), _offset(0), _size(0), _linksOffset(0),
	  _linksSize(0), _shift(0), _linksShift(0) {}
	~NETFile();

	bool load(Handle &hdl, const RGNFile *rgn, Handle &rgnHdl);
	void clear();

	bool lblOffset(Handle &hdl, quint32 netOffset, quint32 &lblOffset) const;
	bool link(const SubDiv *subdiv, quint32 shift, Handle &hdl,
	  const NODFile *nod, Handle &nodHdl2, Handle &nodHdl, const LBLFile *lbl,
	  Handle &lblHdl, const NODFile::BlockInfo &blockInfo, quint8 linkId,
	  quint8 lineId, QList<MapData::Poly> *lines) const;
	bool hasLinks() const {return (_linksSize > 0);}

private:
	bool linkLabel(Handle &hdl, quint32 offset, quint32 size,
	  const LBLFile *lbl, Handle &lblHdl, Label &label) const;

	HuffmanTable *_huffmanTable;
	const HuffmanTable *_tp;
	quint32 _offset, _size, _linksOffset, _linksSize;
	quint8 _shift, _linksShift;
};

}

#endif // IMG_NETFILE_H
