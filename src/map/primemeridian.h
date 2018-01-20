#ifndef PRIMEMERIDIAN_H
#define PRIMEMERIDIAN_H

#include <cmath>
#include <QDebug>

class PrimeMeridian
{
public:
	PrimeMeridian() : _pm(NAN) {}
	PrimeMeridian(int code);

	bool isNull() const {return std::isnan(_pm);}
	bool isValid() const {return !std::isnan(_pm);}

	double toGreenwich(double val) const;
	double fromGreenwich(double val) const;

	friend bool operator==(const PrimeMeridian &pm1, const PrimeMeridian &pm2);
	friend QDebug operator<<(QDebug dbg, const PrimeMeridian &pm);

private:
	double _pm;
};

inline bool operator==(const PrimeMeridian &pm1, const PrimeMeridian &pm2)
  {return (pm1._pm == pm2._pm);}

QDebug operator<<(QDebug dbg, const PrimeMeridian &pm);

#endif // PRIMEMERIDIAN_H
