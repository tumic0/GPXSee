#include "range.h"

void RangeF::resize(qreal size)
{
	qreal adj = (size/2 - this->size()/2);

	_min -= adj;
	_max += adj;
}

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
