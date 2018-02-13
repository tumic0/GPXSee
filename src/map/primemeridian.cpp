#include "primemeridian.h"

static double shift(double lon, double offset)
{
	double ret = lon + offset;

	if (ret > 180.0)
		ret -= 360.0;
	if (ret < -180)
		ret += 360.0;

	return ret;
}

PrimeMeridian::PrimeMeridian(int code)
{
	switch (code) {
		case 8901:
			_pm = 0.0;
			break;
		case 8902:
			_pm = -9.1319061111;
			break;
		case 8903:
			_pm = 2.3372291666;
			break;
		case 8904:
			_pm = -74.0809166666;
			break;
		case 8905:
			_pm = -3.6879388888;
			break;
		case 8906:
			_pm = 12.4523333333;
			break;
		case 8907:
			_pm = 7.4395833333;
			break;
		case 8908:
			_pm = 106.8077194444;
			break;
		case 8909:
			_pm = -17.6666666666;
			break;
		case 8910:
			_pm = 4.3679750000;
			break;
		case 8911:
			_pm = 18.0582777777;
			break;
		case 8913:
			_pm = 10.7229166666;
			break;
		default:
			_pm = NAN;
	}
}

double PrimeMeridian::toGreenwich(double val) const
{
	return shift(val, _pm);
}

double PrimeMeridian::fromGreenwich(double val) const
{
	return shift(val, -_pm);
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const PrimeMeridian &pm)
{
	dbg.nospace() << "PrimeMeridian(" << pm._pm << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
