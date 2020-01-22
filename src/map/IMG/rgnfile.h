#ifndef RGNFILE_H
#define RGNFILE_H

#include "img.h"
#include "subfile.h"
#include "subdiv.h"
#include "huffmantable.h"

class LBLFile;
class NETFile;

class RGNFile : public SubFile
{
public:
	RGNFile(IMG *img)
	  : SubFile(img), _offset(0), _size(0), _polygonsOffset(0),
	  _polygonsSize(0), _linesOffset(0), _linesSize(0), _pointsOffset(0),
	  _pointsSize(0), _init(false) {clearFlags();}
	RGNFile(SubFile *gmp, quint32 offset) : SubFile(gmp, offset), _offset(0),
	  _size(0), _polygonsOffset(0), _polygonsSize(0), _linesOffset(0),
	  _linesSize(0), _pointsOffset(0), _pointsSize(0), _init(false)
	  {clearFlags();}

	void objects(const RectC &rect, const SubDiv *subdiv,
	  LBLFile *lbl, NETFile *net, QList<IMG::Poly> *polygons,
	  QList<IMG::Poly> *lines, QList<IMG::Point> *points);
	void extObjects(const RectC &rect, const SubDiv *subdiv, quint32 shift,
	  LBLFile *lbl, QList<IMG::Poly> *polygons, QList<IMG::Poly> *lines,
	  QList<IMG::Point> *points);

private:
	class Segment {
	public:
		enum Type {
			Point = 0x1,
			IndexedPoint = 0x2,
			Line = 0x4,
			Polygon = 0x8,
			RoadReference = 0x10
		};

		Segment() : _start(0), _end(0), _type(Point) {}
		Segment(quint32 start, Type type)
		  : _start(start), _end(0), _type(type) {}
		Segment(quint32 start, quint32 end, Type type)
		  : _start(start), _end(end), _type(type) {}

		void setEnd(quint32 end) {_end = end;}

		quint32 start() const {return _start;}
		quint32 end() const {return _end;}
		Type type() const {return _type;}

	private:
		quint32 _start;
		quint32 _end;
		Type _type;
	};

	bool init(Handle &hdl);

	QVector<Segment> segments(Handle &hdl, const SubDiv *subdiv) const;
	bool polyObjects(const RectC &rect, Handle &hdl, const SubDiv *subdiv,
	  const Segment &segment, LBLFile *lbl, Handle &lblHdl, NETFile *net,
	  Handle &netHdl, QList<IMG::Poly> *polys, bool line) const;
	bool pointObjects(const RectC &rect, Handle &hdl, const SubDiv *subdiv,
	  const Segment &segment, LBLFile *lbl, Handle &lblHdl,
	  QList<IMG::Point> *points) const;
	bool extPolyObjects(const RectC &rect, Handle &hdl, const SubDiv *subdiv,
	  quint32 shift, const Segment &segment, LBLFile *lbl, Handle &lblHdl,
	  QList<IMG::Poly> *polys, bool line) const;
	bool extPointObjects(const RectC &rect, Handle &hdl, const SubDiv *subdiv,
	  const Segment &segment, LBLFile *lbl, Handle &lblHdl,
	  QList<IMG::Point> *points) const;

	void clearFlags();

	bool skipClassFields(Handle &hdl) const;
	bool skipLclFields(Handle &hdl, const quint32 flags[3],
	  Segment::Type type) const;

	friend QDebug operator<<(QDebug dbg, const RGNFile::Segment &segment);

	quint32 _offset;
	quint32 _size;

	quint32 _polygonsOffset;
	quint32 _polygonsSize;
	quint32 _polygonsFlags[3];
	quint32 _linesOffset;
	quint32 _linesSize;
	quint32 _linesFlags[3];
	quint32 _pointsOffset;
	quint32 _pointsSize;
	quint32 _pointsFlags[3];

	HuffmanTable _huffmanTable;

	bool _init;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const RGNFile::Segment &segment);
#endif // QT_NO_DEBUG

#endif // RGNFILE_H
