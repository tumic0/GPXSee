#ifndef RECTD_H
#define RECTD_H

#include "pointd.h"

class RectD
{
public:
	RectD() {}
	RectD(const PointD &topLeft, const PointD &bottomRight)
	  : _tl(topLeft), _br(bottomRight) {}

	PointD topLeft() const {return _tl;}
	PointD bottomRight() const {return _br;}

	double left() const {return _tl.x();}
	double right() const {return _br.x();}
	double top() const {return _tl.y();}
	double bottom() const {return _br.y();}

	double width() const {return (right() - left());}
	double height() const {return (top() - bottom());}

	bool contains(const PointD &p) const
	  {return (p.x() >= left() && p.x() <= right() && p.y() <= top()
	  && p.y() >= bottom());}

	bool isNull() const {return _tl.isNull() && _br.isNull();}

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
