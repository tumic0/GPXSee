#ifndef IMG_RGNFILE_H
#define IMG_RGNFILE_H

#include "section.h"
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

	RGNFile(const IMGData *img) : SubFile(img), _huffmanTable(0) {}
	RGNFile(const QString &path) : SubFile(path), _huffmanTable(0) {}
	RGNFile(const SubFile *gmp, quint32 offset)
	  : SubFile(gmp, offset), _huffmanTable(0) {}
	~RGNFile();

	void clear();
	bool load(Handle &hdl);

	bool polyObjects(Handle &hdl, const SubDiv *subdiv, SegmentType segmentType,
	  LBLFile *lbl, Handle &lblHdl, NETFile *net, Handle &netHdl,
	  QList<MapData::Poly> *polys) const;
	bool pointObjects(Handle &hdl, const SubDiv *subdiv, SegmentType segmentType,
	  LBLFile *lbl, Handle &lblHdl, QList<MapData::Point> *points) const;
	bool extPolyObjects(Handle &hdl, const SubDiv *subdiv, quint32 shift,
	  SegmentType segmentType, LBLFile *lbl, Handle &lblHdl,
	  QList<MapData::Poly> *polys) const;
	bool extPointObjects(Handle &hdl, const SubDiv *subdiv, LBLFile *lbl,
	  Handle &lblHdl, QList<MapData::Point> *points) const;
	bool links(Handle &hdl, const SubDiv *subdiv, quint32 shift,
	  const NETFile *net, Handle &netHdl, const NODFile *nod, Handle &nodHdl,
	  Handle &nodHdl2, LBLFile *lbl, Handle &lblHdl,
	  QList<MapData::Poly> *lines) const;

	bool subdivInit(Handle &hdl, SubDiv *subdiv) const;

	const HuffmanTable *huffmanTable() const {return _huffmanTable;}
	const Section &dict() const {return _dict;}

private:
	bool segments(Handle &hdl, SubDiv *subdiv, SubDiv::Segment seg[5]) const;
	bool readClassFields(Handle &hdl, SegmentType segmentType, void *object,
	  LBLFile *lbl, Handle &lblHdl) const;
	bool skipLclFields(Handle &hdl, const quint32 flags[3]) const;
	bool skipGblFields(Handle &hdl, quint32 flags) const;
	bool readRasterInfo(Handle &hdl, const LBLFile *lbl, quint32 size,
	  MapData::Poly *poly) const;
	bool readDepthInfo(Handle &hdl, quint8 flags, quint32 size,
	  MapData::Point *point) const;
	bool readObstructionInfo(Handle &hdl, quint8 flags, quint32 size,
	  MapData::Point *point) const;
	bool readBuoyInfo(Handle &hdl, quint8 flags, MapData::Point *point) const;
	bool readLabel(Handle &hdl, LBLFile *lbl, Handle &lblHdl,
	  quint8 flags, quint32 size, MapData::Point *point) const;

	HuffmanTable *_huffmanTable;
	Section _base, _dict, _polygons, _lines, _points;
	quint32 _polygonsGblFlags, _linesGblFlags, _pointsGblFlags;
	quint32 _polygonsLclFlags[3], _linesLclFlags[3], _pointsLclFlags[3];
};

}

#endif // IMG_RGNFILE_H
