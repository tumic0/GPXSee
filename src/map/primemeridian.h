#ifndef PRIMEMERIDIAN_H
#define PRIMEMERIDIAN_H

#include <cmath>
#include <QDebug>

class PrimeMeridian
{
public:
	PrimeMeridian() : _pm(NAN) {}
	PrimeMeridian(int code);
	PrimeMeridian(double lon) : _pm(lon) {}

	bool operator==(const PrimeMeridian &other) const
	  {return fabs(_pm - other._pm) < 1e-6;}

	bool isNull() const {return std::isnan(_pm);}
	bool isValid() const {return !std::isnan(_pm);}

	double toGreenwich(double val) const;
	double fromGreenwich(double val) const;

#ifndef QT_NO_DEBUG
	friend QDebug operator<<(QDebug dbg, const PrimeMeridian &pm);
#endif // QT_NO_DEBUG

private:
	double _pm;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const PrimeMeridian &pm);
#endif // QT_NO_DEBUG

#endif // PRIMEMERIDIAN_H
