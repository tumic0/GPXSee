#ifndef ANGULARUNITS_H
#define ANGULARUNITS_H

#include <cmath>
#include <QDebug>
#include "common/coordinates.h"

class AngularUnits
{
public:
	AngularUnits() : _f(NAN) {}
	AngularUnits(int code);
	AngularUnits(double val) : _f(rad2deg(val)) {}

	bool operator==(const AngularUnits &other) const
	  {return fabs(_f - other._f) < 1e-6;}

	bool isNull() const {return std::isnan(_f);}
	bool isValid() const {return !std::isnan(_f);}

	double toDegrees(double val) const;
	double fromDegrees(double val) const;

#ifndef QT_NO_DEBUG
	friend QDebug operator<<(QDebug dbg, const AngularUnits &au);
#endif // QT_NO_DEBUG

private:
	double _f;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const AngularUnits &au);
#endif // QT_NO_DEBUG

#endif // ANGULARUNITS_H
