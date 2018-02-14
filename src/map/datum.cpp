#include <cmath>
#include "common/wgs84.h"
#include "datum.h"

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

Datum::Datum(const Ellipsoid *ellipsoid, double dx, double dy, double dz)
  : _ellipsoid(ellipsoid), _dx(dx), _dy(dy), _dz(dz)
{
	_WGS84 = (_ellipsoid->radius() == WGS84_RADIUS
	  && _ellipsoid->flattening() == WGS84_FLATTENING && _dx == 0.0
	  && _dy == 0.0 && _dz == 0.0) ? true : false;
}

Coordinates Datum::toWGS84(const Coordinates &c) const
{
	return _WGS84 ? c : molodensky(c, *this, WGS84);
}

Coordinates Datum::fromWGS84(const Coordinates &c) const
{
	return _WGS84 ? c : molodensky(c, WGS84, *this);
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Datum &datum)
{
	dbg.nospace() << "Datum(" << *datum.ellipsoid() << ", " << datum.dx()
	  << ", " << datum.dy() << ", " << datum.dz() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
