#include <cmath>
#include "common/wgs84.h"
#include "datum.h"


#define as2rad(x) ((x) * 4.84814e-6)
#define rad2as(x) ((x) * 206265.0)

static Ellipsoid WGS84e = Ellipsoid(WGS84_RADIUS, WGS84_FLATTENING);
static Datum WGS84 = Datum(&WGS84e, 0.0, 0.0, 0.0);

// Abridged Molodensky transformation
static Coordinates molodensky(const Coordinates &c, const Datum &from,
  const Datum &to)
{
	double rlat = deg2rad(c.lat());
	double rlon = deg2rad(c.lon());

	double slat = sin(rlat);
	double clat = cos(rlat);
	double slon = sin(rlon);
	double clon = cos(rlon);
	double ssqlat = slat * slat;

	double dx = from.dx() - to.dx();
	double dy = from.dy() - to.dy();
	double dz = from.dz() - to.dz();

	double from_f = from.ellipsoid()->flattening();
	double to_f = to.ellipsoid()->flattening();
	double df = to_f - from_f;
	double from_a = from.ellipsoid()->radius();
	double to_a = to.ellipsoid()->radius();
	double da = to_a - from_a;
	double from_esq = from_f * (2.0 - from_f);
	double adb = 1.0 / (1.0 - from_f);
	double rn = from_a / sqrt(1 - from_esq * ssqlat);
	double rm = from_a * (1 - from_esq) / pow((1 - from_esq * ssqlat), 1.5);
	double from_h = 0.0;

	double dlat = (-dx * slat * clon - dy * slat * slon + dz * clat + da
	  * rn * from_esq * slat * clat / from_a + df * (rm * adb + rn / adb) * slat
	  * clat) / (rm + from_h);

	double dlon = (-dx * slon + dy * clon) / ((rn + from_h) * clat);

	return Coordinates(c.lon() + rad2deg(dlon), c.lat() + rad2deg(dlat));
}


Point3D Datum::helmert(const Point3D &p) const
{
	double scale = 1 + _ds * 1e-6;
	double R00 = 1;
	double R01 = _rz;
	double R02 = -_ry;
	double R10 = -_rz;
	double R11 = 1;
	double R12 = _rx;
	double R20 = _ry;
	double R21 = -_rx;
	double R22 = 1;

	return Point3D(scale * (R00 * p.x() + R01 * p.y() + R02 * p.z()) + _dx,
	  scale * (R10 * p.x() + R11 * p.y() + R12 * p.z()) + _dy,
	  scale * (R20 * p.x() + R21 * p.y() + R22 * p.z()) + _dz);
}

Point3D Datum::helmertr(const Point3D &p) const
{
	double scale = 1 + _ds * 1e-6;
	double R00 = 1;
	double R01 = _rz;
	double R02 = -_ry;
	double R10 = -_rz;
	double R11 = 1;
	double R12 = _rx;
	double R20 = _ry;
	double R21 = -_rx;
	double R22 = 1;

	double x = (p.x() - _dx) / scale;
	double y = (p.y() - _dy) / scale;
	double z = (p.z() - _dz) / scale;

	return Point3D(R00 * x + R10 * y + R20 * z, R01 * x + R11 * y + R21 * z,
	  R02 * x + R12 * y + R22 * z);
}

Datum::Datum(const Ellipsoid *ellipsoid, double dx, double dy, double dz,
  double rx, double ry, double rz, double ds)
  : _ellipsoid(ellipsoid), _dx(dx), _dy(dy), _dz(dz), _rx(as2rad(rx)),
  _ry(as2rad(ry)), _rz(as2rad(rz)), _ds(ds)
{
	if (_ellipsoid->radius() == WGS84_RADIUS && _ellipsoid->flattening()
	  == WGS84_FLATTENING && _dx == 0.0 && _dy == 0.0 && _dz == 0.0
	  && _rx == 0.0 && _ry == 0.0 && _rz == 0.0 && _ds == 0.0)
		_shift = None;
	else
		_shift = Helmert;
}

Datum::Datum(const Ellipsoid *ellipsoid, double dx, double dy, double dz)
  : _ellipsoid(ellipsoid), _dx(dx), _dy(dy), _dz(dz), _rx(0.0), _ry(0.0),
  _rz(0.0), _ds(0.0)
{
	if (_ellipsoid->radius() == WGS84_RADIUS && _ellipsoid->flattening()
	  == WGS84_FLATTENING && _dx == 0.0 && _dy == 0.0 && _dz == 0.0)
		_shift = None;
	else
		_shift = Molodensky;
}

Coordinates Datum::toWGS84(const Coordinates &c) const
{
	switch (_shift) {
		case Helmert:
			return Geocentric::toGeodetic(helmert(Geocentric::fromGeodetic(c,
			  *this)), WGS84);
		case Molodensky:
			return molodensky(c, *this, WGS84);
		default:
			return c;
	}
}

Coordinates Datum::fromWGS84(const Coordinates &c) const
{
	switch (_shift) {
		case Helmert:
			return Geocentric::toGeodetic(helmertr(Geocentric::fromGeodetic(c,
			  WGS84)), *this);
		case Molodensky:
			return molodensky(c, WGS84, *this);
		default:
			return c;
	}
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Datum &datum)
{
	dbg.nospace() << "Datum(" << *datum.ellipsoid() << ", " << datum.dx()
	  << ", " << datum.dy() << ", " << datum.dz() << ", " << rad2as(datum.rx())
	  << ", " << rad2as(datum.ry()) << ", " << rad2as(datum.rz()) << ", "
	  << datum.ds() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
