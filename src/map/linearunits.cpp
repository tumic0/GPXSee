#include "linearunits.h"

LinearUnits::LinearUnits(int code)
{
	switch (code) {
		case 9001:
			_f = 1.0;
			break;
		case 9002:
			_f = 0.3048;
			break;
		case 9003:
			_f = 12.0 / 39.37;
			break;
		case 9040:
			_f = 36.0 /	39.370147;
			break;
		case 9041:
			_f = 12.0 / 39.370147;
			break;
		case 9042:
			_f = 792.0 / 39.370147;
			break;
		case 9094:
			_f = 6378300.0 / 20926201.0;
			break;
		default:
			_f = NAN;
	}
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const LinearUnits &lu)
{
	dbg.nospace() << "LinearUnits(" << lu._f << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
