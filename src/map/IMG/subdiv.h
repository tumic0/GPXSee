#ifndef SUBDIV_H
#define SUBDIV_H

#include <QtGlobal>
#include "common/coordinates.h"
#include "common/garmin.h"

class SubDiv {
public:
	class Segment {
	public:
		Segment() : _offset(0), _end(0) {}
		Segment(quint32 ofset, quint32 end) : _offset(ofset), _end(end) {}

		bool isValid() const {return (_end > _offset);}

		quint32 offset() const {return _offset;}
		quint32 end() const {return _end;}

	private:
		quint32 _offset, _end;
	};

	SubDiv(quint32 offset, qint32 lon, qint32 lat, quint8 level, quint8 bits,
	  quint8 objects) : _lon(lon), _lat(lat), _level(level), _bits(bits),
	  _init(false)
	{
		_tre.objects = objects;
		_tre.offset = offset;
		_tre.end = 0;

		_tre.extPolygonsOffset = 0;
		_tre.extPolygonsEnd = 0;
		_tre.extLinesOffset = 0;
		_tre.extLinesEnd = 0;
		_tre.extPointsOffset = 0;
		_tre.extPointsEnd = 0;
	}
	void setEnd(quint32 end) {_tre.end = end;}
	void setExtOffsets(quint32 polygon, quint32 line, quint32 point)
	{
		_tre.extPolygonsOffset = polygon;
		_tre.extLinesOffset = line;
		_tre.extPointsOffset = point;
	}
	void setExtEnds(quint32 polygon, quint32 line, quint32 point)
	{
		_tre.extPolygonsEnd = polygon;
		_tre.extLinesEnd = line;
		_tre.extPointsEnd = point;
	}

	void init(const Segment &points, const Segment &idxPoints,
	  const Segment &lines, const Segment &polygons,
	  const Segment &roadReferences, const Segment &extPoints,
	  const Segment &extLines, const Segment &extPolygons)
	{
		_rgn.pointsOffset = points.offset();
		_rgn.pointsEnd = points.end();
		_rgn.idxPointsOffset = idxPoints.offset();
		_rgn.idxPointsEnd = idxPoints.end();
		_rgn.linesOffset = lines.offset();
		_rgn.linesEnd = lines.end();
		_rgn.polygonsOffset = polygons.offset();
		_rgn.polygonsEnd = polygons.end();
		_rgn.roadReferencesOffset = roadReferences.offset();
		_rgn.roadReferencesEnd = roadReferences.end();
		_rgn.extPointsOffset = extPoints.offset();
		_rgn.extPointsEnd = extPoints.end();
		_rgn.extLinesOffset = extLines.offset();
		_rgn.extLinesEnd = extLines.end();
		_rgn.extPolygonsOffset = extPolygons.offset();
		_rgn.extPolygonsEnd = extPolygons.end();

		_init = true;
	}
	bool initialized() const {return _init;}

	qint32 lon() const {return _lon;}
	qint32 lat() const {return _lat;}
	quint8 bits() const {return _bits;}
	quint8 level() const {return _level;}

	// Valid only after initialization
	Segment points() const
	  {return Segment(_rgn.pointsOffset, _rgn.pointsEnd);}
	Segment idxPoints() const
	  {return Segment(_rgn.idxPointsOffset, _rgn.idxPointsEnd);}
	Segment lines() const
	  {return Segment(_rgn.linesOffset, _rgn.linesEnd);}
	Segment polygons() const
	  {return Segment(_rgn.polygonsOffset, _rgn.polygonsEnd);}
	Segment extPoints() const
	  {return Segment(_rgn.extPointsOffset, _rgn.extPointsEnd);}
	Segment extLines() const
	  {return Segment(_rgn.extLinesOffset, _rgn.extLinesEnd);}
	Segment extPolygons() const
	  {return Segment(_rgn.extPolygonsOffset, _rgn.extPolygonsEnd);}
	Segment roadReferences() const
	  {return Segment(_rgn.roadReferencesOffset, _rgn.roadReferencesEnd);}

	// Valid only until initialization
	quint8 objects() const {return _tre.objects;}
	quint32 offset() const {return _tre.offset;}
	quint32 end() const {return _tre.end;}
	quint32 extPolygonsOffset() const {return _tre.extPolygonsOffset;}
	quint32 extPolygonsEnd() const {return _tre.extPolygonsEnd;}
	quint32 extLinesOffset() const {return _tre.extLinesOffset;}
	quint32 extLinesEnd() const {return _tre.extLinesEnd;}
	quint32 extPointsOffset() const {return _tre.extPointsOffset;}
	quint32 extPointsEnd() const {return _tre.extPointsEnd;}

private:
	struct TRE
	{
		quint8 objects;
		quint32 offset;
		quint32 end;

		quint32 extPolygonsOffset;
		quint32 extPolygonsEnd;
		quint32 extLinesOffset;
		quint32 extLinesEnd;
		quint32 extPointsOffset;
		quint32 extPointsEnd;
	};

	struct RGN
	{
		quint32 pointsOffset;
		quint32 pointsEnd;
		quint32 idxPointsOffset;
		quint32 idxPointsEnd;
		quint32 linesOffset;
		quint32 linesEnd;
		quint32 polygonsOffset;
		quint32 polygonsEnd;
		quint32 roadReferencesOffset;
		quint32 roadReferencesEnd;
		quint32 extPointsOffset;
		quint32 extPointsEnd;
		quint32 extLinesOffset;
		quint32 extLinesEnd;
		quint32 extPolygonsOffset;
		quint32 extPolygonsEnd;
	};

	qint32 _lon, _lat;
	quint8 _level;
	quint8 _bits;
	bool _init;
	union {
		TRE _tre;
		RGN _rgn;
	};
};

#endif // SUBDIV_H
