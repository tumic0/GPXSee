#include "common/wgs84.h"
#include "datum.h"


#define as2rad(x) ((x) * (M_PI/648000.0))
#define rad2as(x) ((x) * (648000.0/M_PI))
#define ds2scale(x) (1.0 + (x) * 1e-6)
#define scale2ds(x) (((x) - 1.0) / 1e-6)

static Ellipsoid WGS84e = Ellipsoid(WGS84_RADIUS, WGS84_FLATTENING);
static Datum WGS84 = Datum(&WGS84e, 0.0, 0.0, 0.0);

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

	double dlat = (-dx * slat * clon - dy * slat * slon + dz * clat + da
	  * rn * from_esq * slat * clat / from_a + df * (rm * adb + rn / adb) * slat
	  * clat) / rm;

	double dlon = (-dx * slon + dy * clon) / (rn * clat);

	return Coordinates(c.lon() + rad2deg(dlon), c.lat() + rad2deg(dlat));
}

Point3D Datum::helmert(const Point3D &p) const
{
	return Point3D(_scale * (p.x() + _rz * p.y() -_ry * p.z()) + _dx,
	  _scale * (-_rz * p.x() + p.y() + _rx * p.z()) + _dy,
	  _scale * (_ry * p.x() -_rx * p.y() + p.z()) + _dz);
}

Point3D Datum::helmertr(const Point3D &p) const
{
	double x = (p.x() - _dx) / _scale;
	double y = (p.y() - _dy) / _scale;
	double z = (p.z() - _dz) / _scale;

	return Point3D(x -_rz * y + _ry * z, _rz * x + y + -_rx * z, -_ry * x + _rx
	  * y + z);
}

Datum::Datum(const Ellipsoid *ellipsoid, double dx, double dy, double dz,
  double rx, double ry, double rz, double ds)
  : _ellipsoid(ellipsoid), _dx(dx), _dy(dy), _dz(dz), _rx(as2rad(rx)),
  _ry(as2rad(ry)), _rz(as2rad(rz)), _scale(ds2scale(ds))
{
	if (_ellipsoid->radius() == WGS84_RADIUS && _ellipsoid->flattening()
	  == WGS84_FLATTENING && _dx == 0.0 && _dy == 0.0 && _dz == 0.0
	  && _rx == 0.0 && _ry == 0.0 && _rz == 0.0 && ds == 0.0)
		_transformation = None;
	else
		_transformation = Helmert;
}

Datum::Datum(const Ellipsoid *ellipsoid, double dx, double dy, double dz)
  : _ellipsoid(ellipsoid), _dx(dx), _dy(dy), _dz(dz), _rx(0.0), _ry(0.0),
  _rz(0.0), _scale(1.0)
{
	if (_ellipsoid->radius() == WGS84_RADIUS && _ellipsoid->flattening()
	  == WGS84_FLATTENING && _dx == 0.0 && _dy == 0.0 && _dz == 0.0)
		_transformation = None;
	else
		_transformation = Molodensky;
}

Coordinates Datum::toWGS84(const Coordinates &c) const
{
	switch (_transformation) {
		case Helmert:
			return Geocentric::toGeodetic(helmert(Geocentric::fromGeodetic(c,
			  ellipsoid())), WGS84.ellipsoid());
		case Molodensky:
			return molodensky(c, *this, WGS84);
		default:
			return c;
	}
}

Coordinates Datum::fromWGS84(const Coordinates &c) const
{
	switch (_transformation) {
		case Helmert:
			return Geocentric::toGeodetic(helmertr(Geocentric::fromGeodetic(c,
			  WGS84.ellipsoid())), ellipsoid());
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
	  << scale2ds(datum.scale()) << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
