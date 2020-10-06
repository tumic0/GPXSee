#ifndef MARGINS_H
#define MARGINS_H

#include <QtGlobal>
#include <QDebug>

class MarginsF
{
public:
	MarginsF() {_left = 0; _top = 0; _right = 0; _bottom = 0;}
	MarginsF(qreal left, qreal top, qreal right, qreal bottom)
	  {_left = left, _top = top; _right = right; _bottom = bottom;}

	qreal left() const {return _left;}
	qreal top() const {return _top;}
	qreal right() const {return _right;}
	qreal bottom() const {return _bottom;}

private:
	qreal _left, _top, _right, _bottom;
};

inline MarginsF operator*(const MarginsF &margins, qreal factor)
{
	return MarginsF(margins.left() * factor, margins.top() * factor,
	  margins.right() * factor, margins.bottom() * factor);
}

inline MarginsF operator/(const MarginsF &margins, qreal factor)
{
	return MarginsF(margins.left() / factor, margins.top() / factor,
	  margins.right() / factor, margins.bottom() / factor);
}

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const MarginsF &margins)
{
	dbg.nospace() << "MarginsF(" << margins.left() << ", " << margins.top()
	  << ", " << margins.right() << margins.bottom() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // MARGINS_H
