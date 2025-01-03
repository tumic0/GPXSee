#ifndef IMG_NETFILE_H
#define IMG_NETFILE_H

#include "section.h"
#include "subfile.h"
#include "nodfile.h"

namespace IMG {

class LBLFile;
class RGNFile;
class SubDiv;
class HuffmanTable;
class BitStream4R;

class NETFile : public SubFile
{
public:
	NETFile(const IMGData *img)
	  : SubFile(img), _huffmanTable(0), _tp(0), _netShift(0), _linksShift(0) {}
	NETFile(const QString &path)
	  : SubFile(path), _huffmanTable(0), _tp(0), _netShift(0), _linksShift(0) {}
	NETFile(const SubFile *gmp, quint32 offset)
	  : SubFile(gmp, offset), _huffmanTable(0), _tp(0), _netShift(0),
	  _linksShift(0) {}
	~NETFile();

	bool load(Handle &hdl, const RGNFile *rgn, Handle &rgnHdl);
	void clear();

	bool lblOffset(Handle &hdl, quint32 netOffset, quint32 &lblOffset) const;
	bool link(const SubDiv *subdiv, quint32 shift, Handle &hdl,
	  const NODFile *nod, Handle &nodHdl2, Handle &nodHdl, LBLFile *lbl,
	  Handle &lblHdl, const NODFile::BlockInfo &blockInfo, quint8 linkId,
	  quint8 lineId, QList<MapData::Poly> *lines) const;
	bool hasLinks() const {return (_links.size > 0);}

private:
	bool linkLabel(Handle &hdl, quint32 offset, LBLFile *lbl,
	  Handle &lblHdl, Label &label) const;
	bool readShape(const NODFile *nod, SubFile::Handle &nodHdl,
	  NODFile::AdjacencyInfo &adj, BitStream4R &bs, const SubDiv *subdiv,
	  quint32 shift, quint16 cnt, bool check, MapData::Poly &poly) const;
	bool readLine(BitStream4R &bs, const SubDiv *subdiv,
	  MapData::Poly &poly) const;

	HuffmanTable *_huffmanTable;
	const HuffmanTable *_tp;
	Section _base, _links;
	quint8 _netShift, _linksShift;
};

}

#endif // IMG_NETFILE_H
