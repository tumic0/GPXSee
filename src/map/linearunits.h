#ifndef LINEARUNITS_H
#define LINEARUNITS_H

#include <cmath>
#include <QDebug>
#include "pointd.h"

class LinearUnits
{
public:
	LinearUnits() : _f(NAN) {}
	LinearUnits(int code);

	bool isNull() const {return std::isnan(_f);}
	bool isValid() const {return !std::isnan(_f);}

	double toMeters(double val) const {return val * _f;}
	PointD toMeters(const PointD &p) const
	  {return PointD(p.x() * _f, p.y() * _f);}
	double fromMeters(double val) const {return val / _f;}
	PointD fromMeters(const PointD &p) const
	  {return PointD(p.x() / _f, p.y() /_f);}

	friend bool operator==(const LinearUnits &lu1, const LinearUnits &lu2);
#ifndef QT_NO_DEBUG
	friend QDebug operator<<(QDebug dbg, const LinearUnits &lu);
#endif // QT_NO_DEBUG

private:
	double _f;
};

inline bool operator==(const LinearUnits &lu1, const LinearUnits &lu2)
  {return (lu1._f == lu2._f);}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const LinearUnits &lu);
#endif // QT_NO_DEBUG

#endif // LINEARUNITS_H
