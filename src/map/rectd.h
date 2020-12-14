#ifndef RECTD_H
#define RECTD_H

#include "pointd.h"

class RectC;
class Projection;

class RectD
{
public:
	RectD() {}
	RectD(const PointD &topLeft, const PointD &bottomRight)
	  : _tl(topLeft), _br(bottomRight) {}
	RectD(const RectC &rect, const Projection &proj, int samples = 100);

	PointD topLeft() const {return _tl;}
	PointD bottomRight() const {return _br;}
	PointD center() const
	  {return PointD((_tl.x() + _br.x()) / 2.0,
		(_tl.y() + _br.y()) / 2.0);}

	double left() const {return _tl.x();}
	double right() const {return _br.x();}
	double top() const {return _tl.y();}
	double bottom() const {return _br.y();}

	void setLeft(double val) {_tl.rx() = val;}
	void setRight(double val) {_br.rx() = val;}
	void setTop(double val) {_tl.ry() = val;}
	void setBottom(double val) {_br.ry() = val;}

	double width() const {return (right() - left());}
	double height() const {return (top() - bottom());}

	bool contains(const PointD &p) const
	  {return (p.x() >= left() && p.x() <= right() && p.y() <= top()
	  && p.y() >= bottom());}

	bool isNull() const {return _tl.isNull() && _br.isNull();}
	bool isValid() const
	  {return (_tl.isValid() && _br.isValid()
	  && _tl.x() != _br.x() && _tl.y() != _br.y());}

	RectC toRectC(const Projection &proj, int samples = 100) const;

private:
	PointD _tl, _br;
};

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const RectD &rect)
{
	dbg.nospace() << "RectD(" << rect.topLeft() << ", " << rect.bottomRight()
	  << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // RECTD_H
