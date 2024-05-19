#ifndef DEMTILE_H
#define DEMTILE_H

#include "common/rectc.h"

namespace IMG {

class DEMTile {
public:
	DEMTile(const RectC &rect, quint32 w, quint32 h, quint32 offset,
	  qint16 base, quint16 diff, quint8 flags)
		: _rect(rect), _w(w), _h(h), _offset(offset), _base(base),
		_diff(diff), _flags(flags) {}

	const RectC &rect() const {return _rect;}
	quint32 w() const {return _w;}
	quint32 h() const {return _h;}

	quint32 offset() const {return _offset;}
	qint16 base() const {return _base;}
	quint16 diff() const {return _diff;}
	quint8 flags() const {return _flags;}

private:
	RectC _rect;
	quint32 _w, _h;

	quint32 _offset;
	qint16 _base;
	quint16 _diff;
	quint8 _flags;
};

}

#endif // DEMTILE_H
