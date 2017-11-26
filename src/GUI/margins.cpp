#include "margins.h"


QDebug operator<<(QDebug dbg, const MarginsF &margins)
{
	dbg.nospace() << "MarginsF(" << margins.left() << ", " << margins.top()
	  << ", " << margins.right() << margins.bottom() << ")";
	return dbg.maybeSpace();
}
