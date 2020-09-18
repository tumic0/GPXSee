#ifndef RGNFILE_H
#define RGNFILE_H

#include "img.h"
#include "subfile.h"
#include "subdiv.h"
#include "huffmantable.h"

class LBLFile;
class NETFile;
class NODFile;

class RGNFile : public SubFile
{
public:
	enum SegmentType {
		Point = 0x1,
		IndexedPoint = 0x2,
		Line = 0x4,
		Polygon = 0x8,
		RoadReference = 0x10
	};

	RGNFile(IMG *img)
	  : SubFile(img), _offset(0), _size(0), _polygonsOffset(0),
	  _polygonsSize(0), _linesOffset(0), _linesSize(0), _pointsOffset(0),
	  _pointsSize(0), _init(false) {}
	RGNFile(const QString &path)
	  : SubFile(path), _offset(0), _size(0), _polygonsOffset(0),
	  _polygonsSize(0), _linesOffset(0), _linesSize(0), _pointsOffset(0),
	  _pointsSize(0), _init(false) {}
	RGNFile(SubFile *gmp, quint32 offset) : SubFile(gmp, offset), _offset(0),
	  _size(0), _polygonsOffset(0), _polygonsSize(0), _linesOffset(0),
	  _linesSize(0), _pointsOffset(0), _pointsSize(0), _init(false) {}

	bool initialized() const {return _init;}
	bool init(Handle &hdl);

	bool polyObjects(Handle &hdl, const SubDiv *subdiv, SegmentType segmentType,
	  LBLFile *lbl, Handle &lblHdl, NETFile *net, Handle &netHdl,
	  QList<IMG::Poly> *polys) const;
	bool pointObjects(Handle &hdl, const SubDiv *subdiv, SegmentType segmentType,
	  LBLFile *lbl, Handle &lblHdl, QList<IMG::Point> *points) const;
	bool extPolyObjects(Handle &hdl, const SubDiv *subdiv, quint32 shift,
	  SegmentType segmentType, LBLFile *lbl, Handle &lblHdl,
	  QList<IMG::Poly> *polys) const;
	bool extPointObjects(Handle &hdl, const SubDiv *subdiv, LBLFile *lbl,
	  Handle &lblHdl, QList<IMG::Point> *points) const;
	bool links(Handle &hdl, const SubDiv *subdiv, quint32 shift, NETFile *net,
	  Handle &netHdl, NODFile *nod, Handle &nodHdl, LBLFile *lbl, Handle &lblHdl,
	  QList<IMG::Poly> *lines) const;

	bool subdivInit(Handle &hdl, SubDiv *subdiv) const;

private:
	QMap<SegmentType, SubDiv::Segment> segments(Handle &hdl, SubDiv *subdiv)
	  const;
	bool skipClassFields(Handle &hdl) const;
	bool skipLclFields(Handle &hdl, const quint32 flags[3])
	  const;
	bool skipGblFields(Handle &hdl, quint32 flags) const;

	quint32 _offset;
	quint32 _size;

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

	HuffmanTable _huffmanTable;

	bool _init;
};

#endif // RGNFILE_H
