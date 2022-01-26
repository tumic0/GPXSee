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
	  {return (_tl.isValid() && _br.isValid()
	  && _tl.lat() != _br.lat() && _tl.lon() != _br.lon());}

	Coordinates topLeft() const {return _tl;}
	Coordinates bottomRight() const {return _br;}
	Coordinates center() const
	  {return Coordinates((_tl.lon() + _br.lon()) / 2.0,
	    (_tl.lat() + _br.lat()) / 2.0);}

	double width() const
	{
		double res = right() - left();
		return (left() > right()) ? 360.0 - res : res;
	}
	double height() const {return (top() - bottom());}
	double area() const {return qAbs(width()) * qAbs(height());}

	double top() const {return _tl.lat();}
	double bottom() const {return _br.lat();}
	double left() const {return _tl.lon();}
	double right() const {return _br.lon();}

	void setLeft(double val) {_tl.rlon() = val;}
	void setRight(double val) {_br.rlon() = val;}
	void setTop(double val) {_tl.rlat() = val;}
	void setBottom(double val) {_br.rlat() = val;}

	RectC operator|(const RectC &r) const;
	RectC &operator|=(const RectC &r) {*this = *this | r; return *this;}
	RectC operator&(const RectC &r) const;
	RectC &operator&=(const RectC &r) {*this = *this & r; return *this;}

	RectC united(const Coordinates &c) const;
	RectC adjusted(double lon1, double lat1, double lon2, double lat2) const;

	bool intersects(const RectC &r) const
	  {return (right() >= r.left() && bottom() <= r.top() && left() <= r.right()
		&& top() >= r.bottom());}
	bool contains(const Coordinates&c) const
	  {return (c.lon() >= left() && c.lon() <= right() && c.lat() <= top()
		&& c.lat() >= bottom());}

private:
	Coordinates _tl, _br;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const RectC &rect);
#endif // QT_NO_DEBUG

#endif // RECTC_H
