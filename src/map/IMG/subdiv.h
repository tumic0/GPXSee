#ifndef SUBDIV_H
#define SUBDIV_H

#include <QtGlobal>
#include "common/coordinates.h"
#include "common/garmin.h"

class SubDiv {
public:
	class Segment {
	public:
		Segment() : _start(0), _end(0) {}
		Segment(quint32 start, quint32 end) : _start(start), _end(end) {}

		bool isValid() const {return (_end > _start);}

		quint32 start() const {return _start;}
		quint32 end() const {return _end;}

	private:
		quint32 _start, _end;
	};

	SubDiv(quint32 offset, qint32 lon, qint32 lat, int bits, quint8 objects)
	  : _lon(lon), _lat(lat), _bits(bits), _init(false)
	{
		_tre.objects = objects;
		_tre.offset = offset;
		_tre.end = 0;

		_tre.polygonsOffset = 0;
		_tre.polygonsEnd = 0;
		_tre.linesOffset = 0;
		_tre.linesEnd = 0;
		_tre.pointsOffset = 0;
		_tre.pointsEnd = 0;
	}
	void setEnd(quint32 end) {_tre.end = end;}
	void setExtOffsets(quint32 polygon, quint32 line, quint32 point)
	{
		_tre.polygonsOffset = polygon;
		_tre.linesOffset = line;
		_tre.pointsOffset = point;
	}
	void setExtEnds(quint32 polygon, quint32 line, quint32 point)
	{
		_tre.polygonsEnd = polygon;
		_tre.linesEnd = line;
		_tre.pointsEnd = point;
	}

	void init(const Segment &points, const Segment &idxPoints,
	  const Segment &lines, const Segment &polygons,
	  const Segment &roadReferences, const Segment &extPoints,
	  const Segment &extLines, const Segment &extPolygons)
	{
		_rgn.points = points;
		_rgn.idxPoints = idxPoints;
		_rgn.lines = lines;
		_rgn.polygons = polygons;
		_rgn.roadReferences = roadReferences;
		_rgn.extPoints = extPoints;
		_rgn.extLines = extLines;
		_rgn.extPolygons = extPolygons;
		_init = true;
	}
	bool initialized() const {return _init;}

	qint32 lon() const {return _lon;}
	qint32 lat() const {return _lat;}
	quint8 bits() const {return _bits;}

	// Valid only after initialization
	const Segment &points() const {return _rgn.points;}
	const Segment &idxPoints() const {return _rgn.idxPoints;}
	const Segment &lines() const {return _rgn.lines;}
	const Segment &polygons() const {return _rgn.polygons;}
	const Segment &extPoints() const {return _rgn.extPoints;}
	const Segment &extLines() const {return _rgn.extLines;}
	const Segment &extPolygons() const {return _rgn.extPolygons;}

	// Valid only until initialization
	quint8 objects() const {return _tre.objects;}
	quint32 offset() const {return _tre.offset;}
	quint32 end() const {return _tre.end;}
	quint32 extPolygonsOffset() const {return _tre.polygonsOffset;}
	quint32 extPolygonsEnd() const {return _tre.polygonsEnd;}
	quint32 extLinesOffset() const {return _tre.linesOffset;}
	quint32 extLinesEnd() const {return _tre.linesEnd;}
	quint32 extPointsOffset() const {return _tre.pointsOffset;}
	quint32 extPointsEnd() const {return _tre.pointsEnd;}

private:
	struct TRE
	{
		quint8 objects;
		quint32 offset;
		quint32 end;

		quint32 polygonsOffset;
		quint32 polygonsEnd;
		quint32 linesOffset;
		quint32 linesEnd;
		quint32 pointsOffset;
		quint32 pointsEnd;
	};

	struct RGN
	{
		Segment points;
		Segment idxPoints;
		Segment lines;
		Segment polygons;
		Segment roadReferences;
		Segment extPoints;
		Segment extLines;
		Segment extPolygons;
	};

	qint32 _lon, _lat;
	quint8 _bits;
	bool _init;
	union {
		TRE _tre;
		RGN _rgn;
	};
};

#endif // SUBDIV_H
