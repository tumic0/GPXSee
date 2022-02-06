#ifndef IMG_RGNFILE_H
#define IMG_RGNFILE_H

#include "subfile.h"
#include "subdiv.h"

namespace IMG {

class LBLFile;
class NETFile;
class NODFile;
class HuffmanTable;

class RGNFile : public SubFile
{
public:
	enum SegmentType {
		Point,
		IndexedPoint,
		Line,
		Polygon,
		RoadReference
	};

	RGNFile(const IMGData *img)
	  : SubFile(img), _huffmanTable(0), _offset(0), _size(0), _polygonsOffset(0),
	  _polygonsSize(0), _linesOffset(0), _linesSize(0), _pointsOffset(0),
	  _pointsSize(0) {}
	RGNFile(const QString *path)
	  : SubFile(path), _huffmanTable(0), _offset(0), _size(0), _polygonsOffset(0),
	  _polygonsSize(0), _linesOffset(0), _linesSize(0), _pointsOffset(0),
	  _pointsSize(0) {}
	RGNFile(const SubFile *gmp, quint32 offset) : SubFile(gmp, offset),
	  _huffmanTable(0), _offset(0), _size(0), _polygonsOffset(0),
	  _polygonsSize(0), _linesOffset(0), _linesSize(0), _pointsOffset(0),
	  _pointsSize(0) {}
	~RGNFile();

	void clear();
	bool load(Handle &hdl);

	bool polyObjects(Handle &hdl, const SubDiv *subdiv, SegmentType segmentType,
	  const LBLFile *lbl, Handle &lblHdl, NETFile *net, Handle &netHdl,
	  QList<MapData::Poly> *polys) const;
	bool pointObjects(Handle &hdl, const SubDiv *subdiv, SegmentType segmentType,
	  const LBLFile *lbl, Handle &lblHdl, QList<MapData::Point> *points) const;
	bool extPolyObjects(Handle &hdl, const SubDiv *subdiv, quint32 shift,
	  SegmentType segmentType, const LBLFile *lbl, Handle &lblHdl,
	  QList<MapData::Poly> *polys) const;
	bool extPointObjects(Handle &hdl, const SubDiv *subdiv, const LBLFile *lbl,
	  Handle &lblHdl, QList<MapData::Point> *points) const;
	bool links(Handle &hdl, const SubDiv *subdiv, quint32 shift,
	  const NETFile *net, Handle &netHdl, const NODFile *nod, Handle &nodHdl,
	  Handle &nodHdl2, const LBLFile *lbl, Handle &lblHdl,
	  QList<MapData::Poly> *lines) const;

	bool subdivInit(Handle &hdl, SubDiv *subdiv) const;

	const HuffmanTable *huffmanTable() const {return _huffmanTable;}
	quint32 dictOffset() const {return _dictOffset;}
	quint32 dictSize() const {return _dictSize;}

private:
	bool segments(Handle &hdl, SubDiv *subdiv, SubDiv::Segment seg[5]) const;
	bool readClassFields(Handle &hdl, SegmentType segmentType, void *object,
	  const LBLFile *lbl) const;
	bool skipLclFields(Handle &hdl, const quint32 flags[3]) const;
	bool skipGblFields(Handle &hdl, quint32 flags) const;

	HuffmanTable *_huffmanTable;

	quint32 _offset;
	quint32 _size;
	quint32 _dictOffset;
	quint32 _dictSize;

	quint32 _polygonsOffset;
	quint32 _polygonsSize;
	quint32 _polygonsLclFlags[3];
	quint32 _polygonsGblFlags;
	quint32 _linesOffset;
	quint32 _linesSize;
	quint32 _linesLclFlags[3];
	quint32 _linesGblFlags;
	quint32 _pointsOffset;
	quint32 _pointsSize;
	quint32 _pointsLclFlags[3];
	quint32 _pointsGblFlags;
};

}

#endif // IMG_RGNFILE_H
