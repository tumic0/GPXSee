#ifndef DATUM_H
#define DATUM_H

#include <cmath>
#include <QDebug>
#include "common/coordinates.h"
#include "ellipsoid.h"
#include "geocentric.h"

class Datum
{
public:
	Datum() : _ellipsoid(0), _transformation(None), _dx(NAN), _dy(NAN),
	  _dz(NAN), _rx(NAN), _ry(NAN), _rz(NAN), _scale(NAN) {}
	Datum(const Ellipsoid *ellipsoid, double dx, double dy, double dz,
	  double rx, double ry, double rz, double ds);
	Datum(const Ellipsoid *ellipsoid, double dx, double dy, double dz);

	const Ellipsoid *ellipsoid() const {return _ellipsoid;}
	double dx() const {return _dx;}
	double dy() const {return _dy;}
	double dz() const {return _dz;}
	double rx() const {return _rx;}
	double ry() const {return _ry;}
	double rz() const {return _rz;}
	double scale() const {return _scale;}

	bool isNull() const
	  {return !_ellipsoid;}
	bool isValid() const
	  {return (_ellipsoid && !std::isnan(_dx) && !std::isnan(_dy)
		&& !std::isnan(_dz) && !std::isnan(_scale) && !std::isnan(_rx)
		&& !std::isnan(_ry) && !std::isnan(_rz));}

	Coordinates toWGS84(const Coordinates &c) const;
	Coordinates fromWGS84(const Coordinates &c) const;

	static const Datum &WGS84();

private:
	enum TransformationType {
		None,
		Molodensky,
		Helmert
	};

	Point3D helmert(const Point3D &p) const;
	Point3D helmertr(const Point3D &p) const;

	const Ellipsoid *_ellipsoid;
	TransformationType _transformation;
	double _dx, _dy, _dz, _rx, _ry, _rz, _scale;
};

inline bool operator==(const Datum &d1, const Datum &d2)
  {return (*d1.ellipsoid() == *d2.ellipsoid() && d1.dx() == d2.dx()
	&& d1.dy() == d2.dy() && d1.dz() == d2.dz() && d1.rx() == d2.rx()
	&& d1.ry() == d2.ry() && d1.rz() == d2.rz() && d1.scale() == d2.scale());}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Datum &datum);
#endif // QT_NO_DEBUG

#endif // DATUM_H
