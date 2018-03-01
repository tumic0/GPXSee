#include "common/coordinates.h"
#include "common/str2int.h"
#include "angularunits.h"

static double sDMS2deg(double val)
{
	double angle;
	const char *decimal;

	QString qstr(QString::number(qAbs(val), 'f', 7));
	QByteArray ba = qstr.toLatin1();
	const char *str = ba.constData();
	decimal = strrchr(str, '.');
	int deg = str2int(str, decimal - str);
	int min = str2int(decimal + 1, 2);
	int sec = str2int(decimal + 3, 2);
	int f = str2int(decimal + 5, 3);

	angle = deg + min/60.0 + sec/3600.0 + (f/1000.0)/3600.0;

	return (val < 0) ? -angle : angle;
}

static double deg2sDMS(double val)
{
	double aval = qAbs(val);

	int deg = aval;
	double r1 = aval - deg;

	int min = r1 * 60.0;
	double r2 = r1 - (min / 60.0);

	int sec = r2 * 3600.0;
	double r3 = r2 - (sec / 3600.0);
	int f = (int)(r3 * 3600.0 * 1000.0);

	QString str(QString("%1.%2%3%4").arg(deg).arg(min, 2, 10, QChar('0'))
	  .arg(sec, 2, 10, QChar('0')).arg(f, 3, 10, QChar('0')));

	return (val < 0) ? -str.toDouble() : str.toDouble();
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

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const AngularUnits &au)
{
	dbg.nospace() << "AngularUnits(" << deg2rad(au._f) << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
