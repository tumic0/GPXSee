#ifndef ANGULARUNITS_H
#define ANGULARUNITS_H

#include <cmath>
#include <QDebug>

class AngularUnits
{
public:
	AngularUnits() : _code(0), _f(NAN) {}
	AngularUnits(int code);

	bool isNull() const {return std::isnan(_f);}
	bool isValid() const {return !std::isnan(_f);}

	double toDegrees(double val) const;
	double fromDegrees(double val) const;

	friend bool operator==(const AngularUnits &au1, const AngularUnits &au2);
#ifndef QT_NO_DEBUG
	friend QDebug operator<<(QDebug dbg, const AngularUnits &au);
#endif // QT_NO_DEBUG
private:
	int _code;
	double _f;
};

inline bool operator==(const AngularUnits &au1, const AngularUnits &au2)
  {return (au1._f == au2._f);}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const AngularUnits &au);
#endif // QT_NO_DEBUG

#endif // ANGULARUNITS_H
