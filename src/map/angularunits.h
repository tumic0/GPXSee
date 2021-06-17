#ifndef ANGULARUNITS_H
#define ANGULARUNITS_H

#include <cmath>
#include <QDebug>
#include "common/coordinates.h"

class AngularUnits
{
public:
	AngularUnits() : _code(0), _f(NAN) {}
	AngularUnits(int code);
	AngularUnits(double val) : _code(0), _f(rad2deg(val)) {}

	bool operator==(const AngularUnits &other) const
	{
		if (_code == 9110)
			return (other._code == 9110);
		else
			return (_f == other._f);
	}

	bool isNull() const {return std::isnan(_f);}
	bool isValid() const {return !std::isnan(_f);}

	double toDegrees(double val) const;
	double fromDegrees(double val) const;

#ifndef QT_NO_DEBUG
	friend QDebug operator<<(QDebug dbg, const AngularUnits &au);
#endif // QT_NO_DEBUG
private:
	int _code;
	double _f;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const AngularUnits &au);
#endif // QT_NO_DEBUG

#endif // ANGULARUNITS_H
