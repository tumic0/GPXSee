#include "common/coordinates.h"
#include "angularunits.h"

AngularUnits::AngularUnits(int code)
{
	switch (code) {
		case 9101:
			_f = 180.0 / M_PI;
			break;
		case 9102:
		case 9107:
		case 9108:
		case 9110:
		case 9122:
			_f = 1.0;
			break;
		case 9103:
			_f = 1 / 60.0;
			break;
		case 9104:
			_f = 1 / 3600.0;
			break;
		case 9105:
			_f = 180.0 / 200.0;
			break;
		case 9106:
			_f = 180.0 / 200.0;
			break;
		case 9109:
			_f = 180.0 / (M_PI * 1000000.0);
			break;
		default:
			_f = NAN;
	}
}

QDebug operator<<(QDebug dbg, const AngularUnits &au)
{
	dbg.nospace() << "AngularUnits(" << deg2rad(au._f) << ")";
	return dbg.maybeSpace();
}
