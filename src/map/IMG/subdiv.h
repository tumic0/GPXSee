#ifndef SUBDIV_H
#define SUBDIV_H

#include <QtGlobal>
#include "common/coordinates.h"
#include "units.h"

class SubDiv {
public:
	SubDiv(quint32 offset, qint32 lon, qint32 lat, int bits, quint8 objects)
	  : _offset(offset), _end(0), _lon(lon), _lat(lat), _bits(bits),
	  _objects(objects), _polygonsOffset(0), _polygonsEnd(0), _linesOffset(0),
	  _linesEnd(0), _pointsOffset(0), _pointsEnd(0) {}
	void setEnd(quint32 end) {_end = end;}

	quint32 offset() const {return _offset;}
	quint32 end() const {return _end;}
	qint32 lon() const {return _lon;}
	qint32 lat() const {return _lat;}
	quint8 bits() const {return _bits;}
	quint8 objects() const {return _objects;}

	// Extended types objects (TRE7)
	void setExtOffsets(quint32 polygon, quint32 line, quint32 point)
	  {_polygonsOffset = polygon; _linesOffset = line; _pointsOffset = point;}
	void setExtEnds(quint32 polygon, quint32 line, quint32 point)
	  {_polygonsEnd = polygon; _linesEnd = line; _pointsEnd = point;}

	quint32 polygonsOffset() const {return _polygonsOffset;}
	quint32 polygonsEnd() const {return _polygonsEnd;}
	quint32 linesOffset() const {return _linesOffset;}
	quint32 linesEnd() const {return _linesEnd;}
	quint32 pointsOffset() const {return _pointsOffset;}
	quint32 pointsEnd() const {return _pointsEnd;}

private:
	quint32 _offset;
	quint32 _end;
	qint32 _lon, _lat;
	quint8 _bits;
	quint8 _objects;

	quint32 _polygonsOffset;
	quint32 _polygonsEnd;
	quint32 _linesOffset;
	quint32 _linesEnd;
	quint32 _pointsOffset;
	quint32 _pointsEnd;
};

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const SubDiv &subdiv)
{
	Coordinates c(toWGS84(subdiv.lon()), toWGS84(subdiv.lat()));
	dbg.nospace() << "SubDiv(" << c << ", " << subdiv.offset()
	  << ", " << subdiv.end() << ", " << subdiv.objects() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // SUBDIV_H
