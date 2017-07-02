#ifndef RECTC_H
#define RECTC_H

#include <QDebug>
#include <QSizeF>
#include "coordinates.h"

class RectC
{
public:
	RectC() {}
	RectC(const Coordinates &topLeft, const Coordinates &bottomRight)
	  : _tl(topLeft), _br(bottomRight) {}

	bool isNull() const
	  {return _tl.isNull() && _br.isNull();}
	bool isValid() const
	  {return (_tl.isValid() && _br.isValid() && _tl != _br);}

	Coordinates topLeft() const {return _tl;}
	Coordinates bottomRight() const {return _br;}

	Coordinates center() const
	  {return Coordinates((_tl.lon() + _br.lon()) / 2.0,
	    (_tl.lat() + _br.lat()) / 2.0);}
	qreal width() const {return  _br.lon() - _tl.lon();}
	qreal height() const {return  _br.lat() - _tl.lat();}

	QSizeF size() const {return QSizeF(width(), height());}

	RectC operator|(const RectC &r) const;
	RectC &operator|=(const RectC &r) {*this = *this | r; return *this;}

	void unite(const Coordinates &c);

private:
	Coordinates _tl, _br;
};

QDebug operator<<(QDebug dbg, const RectC &rect);

#endif // RECTC_H
