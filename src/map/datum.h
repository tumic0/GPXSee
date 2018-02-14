#ifndef DATUM_H
#define DATUM_H

#include <cmath>
#include <QList>
#include <QDebug>
#include "ellipsoid.h"
#include "common/coordinates.h"

class Datum
{
public:
	Datum() : _ellipsoid(0), _dx(NAN), _dy(NAN), _dz(NAN),
	  _WGS84(false) {}
	Datum(const Ellipsoid *ellipsoid, double dx, double dy, double dz);

	const Ellipsoid *ellipsoid() const {return _ellipsoid;}
	double dx() const {return _dx;}
	double dy() const {return _dy;}
	double dz() const {return _dz;}

	bool isNull() const
	  {return (!_ellipsoid && std::isnan(_dx) && std::isnan(_dy)
	    && std::isnan(_dz));}
	bool isValid() const
	  {return (_ellipsoid && !std::isnan(_dx) && !std::isnan(_dy)
	    && !std::isnan(_dz));}

	Coordinates toWGS84(const Coordinates &c) const;
	Coordinates fromWGS84(const Coordinates &c) const;

private:
	const Ellipsoid *_ellipsoid;
	double _dx, _dy, _dz;
	bool _WGS84;
};

inline bool operator==(const Datum &d1, const Datum &d2)
  {return (d1.ellipsoid() == d2.ellipsoid() && d1.dx() == d2.dx()
    && d1.dy() == d2.dy() && d1.dz() == d2.dz());}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Datum &datum);
#endif // QT_NO_DEBUG

#endif // DATUM_H
