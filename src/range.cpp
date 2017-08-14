#include "range.h"

void RangeF::resize(qreal size)
{
	qreal adj = (size/2 - this->size()/2);

	_min -= adj;
	_max += adj;
}

QDebug operator<<(QDebug dbg, const RangeF &range)
{
	const bool ais = dbg.autoInsertSpaces();
	dbg.nospace() << "RangeF(" << range.min() << ", " << range.max() << ")";
	dbg.setAutoInsertSpaces(ais);
	return dbg.maybeSpace();
}
