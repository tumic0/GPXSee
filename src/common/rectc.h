#ifndef RECTC_H
#define RECTC_H

#include <QDebug>
#include "coordinates.h"

class RectC
{
public:
	RectC() {}
	RectC(const Coordinates &topLeft, const Coordinates &bottomRight)
	  : _tl(topLeft), _br(bottomRight) {}
	RectC(const Coordinates &center, double radius);

	bool isNull() const
	  {return _tl.isNull() && _br.isNull();}
	bool isValid() const
	  {return (_tl.isValid() && _br.isValid() && _tl != _br);}

	Coordinates topLeft() const {return _tl;}
	Coordinates bottomRight() const {return _br;}
	Coordinates center() const
	  {return Coordinates((_tl.lon() + _br.lon()) / 2.0,
	    (_tl.lat() + _br.lat()) / 2.0);}

	RectC operator|(const RectC &r) const;
	RectC &operator|=(const RectC &r) {*this = *this | r; return *this;}
	RectC operator&(const RectC &r) const;
	RectC &operator&=(const RectC &r) {*this = *this & r; return *this;}

	RectC united(const Coordinates &c) const;

private:
	Coordinates _tl, _br;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const RectC &rect);
#endif // QT_NO_DEBUG

#endif // RECTC_H
