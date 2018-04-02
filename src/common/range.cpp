#include "range.h"

void RangeF::resize(qreal size)
{
	qreal adj = (size/2 - this->size()/2);

	_min -= adj;
	_max += adj;
}

RangeF RangeF::operator&(const RangeF &r) const
{
	if (isNull() || r.isNull())
		return RangeF();

	RangeF tmp(qMax(this->_min, r._min), qMin(this->_max, r._max));
	return tmp.isValid() ? tmp : RangeF();
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Range &range)
{
	dbg.nospace() << "Range(" << range.min() << ", " << range.max() << ")";
	return dbg.space();
}

QDebug operator<<(QDebug dbg, const RangeF &range)
{
	dbg.nospace() << "RangeF(" << range.min() << ", " << range.max() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
