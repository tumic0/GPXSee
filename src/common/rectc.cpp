#include "wgs84.h"
#include "rectc.h"

#define MIN_LAT deg2rad(-90.0)
#define MAX_LAT deg2rad(90.0)
#define MIN_LON deg2rad(-180.0)
#define MAX_LON deg2rad(180.0)

RectC::RectC(const Coordinates &center, double radius)
{
	double radDist = radius / WGS84_RADIUS;
	double radLon = deg2rad(center.lon());
	double radlat = deg2rad(center.lat());

	double minLat = radlat - radDist;
	double maxLat = radlat + radDist;

	double minLon, maxLon;
	if (minLat > MIN_LAT && maxLat < MAX_LAT) {
		double deltaLon = asin(sin(radDist) / cos(radlat));
		minLon = radLon - deltaLon;
		if (minLon < MIN_LON)
			minLon += M_2_PI;
		maxLon = radLon + deltaLon;
		if (maxLon > MAX_LON)
			maxLon -= M_2_PI;
	} else {
		// a pole is within the distance
		minLat = qMax(minLat, MIN_LAT);
		maxLat = qMin(maxLat, MAX_LAT);
		minLon = MIN_LON;
		maxLon = MAX_LON;
	}

	_tl = Coordinates(rad2deg(minLon), rad2deg(maxLat));
	_br = Coordinates(rad2deg(maxLon), rad2deg(minLat));
}

RectC RectC::operator|(const RectC &r) const
{
	if (isNull())
		return r;
	if (r.isNull())
		return *this;

	double l1 = _tl.lon();
	double r1 = _tl.lon();
	if (_br.lon() < _tl.lon())
		l1 = _br.lon();
	else
		r1 = _br.lon();

	double l2 = r._tl.lon();
	double r2 = r._tl.lon();
	if (r._br.lon() < r._tl.lon())
		l2 = r._br.lon();
	else
		r2 = r._br.lon();

	double t1 = _tl.lat();
	double b1 = _tl.lat();
	if (_br.lat() > _tl.lat())
		t1 = _br.lat();
	else
		b1 = _br.lat();

	double t2 = r._tl.lat();
	double b2 = r._tl.lat();
	if (r._br.lat() > r._tl.lat())
		t2 = r._br.lat();
	else
		b2 = r._br.lat();

	return RectC(Coordinates(qMin(l1, l2), qMax(t1, t2)),
	  Coordinates(qMax(r1, r2), qMin(b1, b2)));
}

RectC RectC::operator&(const RectC &r) const
{
	if (isNull() || r.isNull())
		return RectC();

	double l1 = _tl.lon();
	double r1 = _tl.lon();
	if (_br.lon() < _tl.lon())
		l1 = _br.lon();
	else
		r1 = _br.lon();

	double l2 = r._tl.lon();
	double r2 = r._tl.lon();
	if (r._br.lon() < r._tl.lon())
		l2 = r._br.lon();
	else
		r2 = r._br.lon();

	if (l1 > r2 || l2 > r1)
		return RectC();

	double t1 = _tl.lat();
	double b1 = _tl.lat();
	if (_br.lat() > _tl.lat())
		t1 = _br.lat();
	else
		b1 = _br.lat();

	double t2 = r._tl.lat();
	double b2 = r._tl.lat();
	if (r._br.lat() > r._tl.lat())
		t2 = r._br.lat();
	else
		b2 = r._br.lat();

	if (b1 > t2 || b2 > t1)
		return RectC();

	return RectC(Coordinates(qMax(l1, l2), qMin(t1, t2)),
	  Coordinates(qMin(r1, r2), qMax(b1, b2)));
}

RectC RectC::united(const Coordinates &c) const
{
	if (c.isNull())
		return *this;
	if (isNull())
		return RectC(c, c);

	double l = _tl.lon();
	double r = _tl.lon();
	if (_br.lon() < _tl.lon())
		l = _br.lon();
	else
		r = _br.lon();

	double t = _tl.lat();
	double b = _tl.lat();
	if (_br.lat() > _tl.lat())
		t = _br.lat();
	else
		b = _br.lat();

	if (c.lon() < l)
		l = c.lon();
	if (c.lon() > r)
		r = c.lon();
	if (c.lat() < b)
		b = c.lat();
	if (c.lat() > t)
		t = c.lat();

	return RectC(Coordinates(l, t), Coordinates(r, b));
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const RectC &rect)
{
	dbg.nospace() << "RectC(" << rect.topLeft() << ", " << rect.bottomRight()
	  << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
