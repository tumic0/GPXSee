#ifndef RGNFILE_H
#define RGNFILE_H

#include "img.h"
#include "subfile.h"
#include "subdiv.h"

class LBLFile;
class NETFile;

class RGNFile : public SubFile
{
public:
	RGNFile(IMG *img, quint32 size)
	  : SubFile(img, size), _offset(0), _size(0), _polygonsOffset(0),
	  _polygonsSize(), _linesOffset(), _linesSize(), _pointsOffset(),
	  _pointsSize() {}

	void objects(const RectC &rect, const SubDiv *subdiv, LBLFile *lbl,
	  NETFile *net, QList<IMG::Poly> *polygons, QList<IMG::Poly> *lines,
	  QList<IMG::Point> *points);
	void extObjects(const RectC &rect, const SubDiv *subdiv, LBLFile *lbl,
	  QList<IMG::Poly> *polygons, QList<IMG::Poly> *lines,
	  QList<IMG::Point> *points);

private:
	class Segment {
	public:
		enum Type {
			Point = 0x10,
			IndexedPoint = 0x20,
			Line = 0x40,
			Polygon = 0x80
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

	class BitStream {
	public:
		BitStream(const SubFile &file, Handle &hdl, quint16 length)
		  : _file(file), _hdl(hdl), _length(length), _remaining(0) {}

		bool read(int bits, quint32 &val);
		bool readDelta(int bits, int sign, bool extraBit, qint32 &delta);
		bool hasNext(int bits) const
		  {return _length * 8 + _remaining >= (quint32)bits;}
		bool finish();

	private:
		const SubFile &_file;
		Handle &_hdl;
		quint16 _length;
		quint32 _remaining;
		quint8 _data;
	};

	static bool sign(BitStream &bs, int &val);
	static int bitSize(quint8 baseSize, bool variableSign, bool extraBit);

	bool init();
	QVector<Segment> segments(Handle &hdl, const SubDiv *subdiv) const;
	bool polyObjects(const RectC &rect, Handle &hdl, const SubDiv *subdiv,
	  const Segment &segment, LBLFile *lbl, Handle &lblHdl, NETFile *net,
	  Handle &netHdl, QList<IMG::Poly> *polys) const;
	bool pointObjects(const RectC &rect, Handle &hdl, const SubDiv *subdiv,
	  const Segment &segment, LBLFile *lbl, Handle &lblHdl,
	  QList<IMG::Point> *points) const;
	bool extPolyObjects(const RectC &rect, Handle &hdl, const SubDiv *subdiv,
	  const Segment &segment, LBLFile *lbl, Handle &lblHdl,
	  QList<IMG::Poly> *polys) const;
	bool extPointObjects(const RectC &rect, Handle &hdl, const SubDiv *subdiv,
	  const Segment &segment, LBLFile *lbl, Handle &lblHdl,
	  QList<IMG::Point> *points) const;

	friend QDebug operator<<(QDebug dbg, const RGNFile::Segment &segment);

	quint32 _offset;
	quint32 _size;

	quint32 _polygonsOffset;
	quint32 _polygonsSize;
	quint32 _linesOffset;
	quint32 _linesSize;
	quint32 _pointsOffset;
	quint32 _pointsSize;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const RGNFile::Segment &segment);
#endif // QT_NO_DEBUG

#endif // RGNFILE_H
