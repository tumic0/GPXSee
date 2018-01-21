#include "common/coordinates.h"
#include "data/str2int.h"
#include "angularunits.h"

static double sDMS2deg(double val)
{
	double angle;
	char *decimal;
	char str[13];

	if (val < -999.9 || val > 999.9)
		return NAN;

	sprintf(str, "%.7f", qAbs(val));
	decimal = strchr(str,'.');
	int deg = str2int(str, decimal - str);
	int min = str2int(decimal + 1, 2);
	int sec = str2int(decimal + 3, 2);
	int f = str2int(decimal + 5, 3);

	angle = deg + (double)min/60.0 + (double)sec/3600.0
	  + ((double)f/1000.0)/3600.0;

	return (val < 0) ? -angle : angle;
}

static double deg2sDMS(double val)
{
	char str[13];
	double aval = qAbs(val);

	if (val < -999.9 || val > 999.9)
		return NAN;

	int deg = aval;
	double r1 = aval - deg;

	int min = r1 * 60.0;
	double r2 = r1 - (min / 60.0);

	int sec = r2 * 3600.0;
	double r3 = r2 - (sec / 3600.0);
	int f = (int)(r3 * 3600.0 * 1000.0);

	sprintf(str, "%u.%02u%02u%03u", deg, min, sec, f);
	return (val < 0) ? -atof(str) : atof(str);
}

AngularUnits::AngularUnits(int code) : _code(code)
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

double AngularUnits::toDegrees(double val) const
{
	return (_code == 9110) ? sDMS2deg(val) : val * _f;
}

double AngularUnits::fromDegrees(double val) const
{
	return (_code == 9110) ? deg2sDMS(val) : val / _f;
}

QDebug operator<<(QDebug dbg, const AngularUnits &au)
{
	dbg.nospace() << "AngularUnits(" << deg2rad(au._f) << ")";
	return dbg.space();
}
