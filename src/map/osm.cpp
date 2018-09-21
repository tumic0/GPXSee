#include "osm.h"

#define EPSILON       1e-6

QPointF osm::ll2m(const Coordinates &c)
{
	return QPointF(c.lon(), rad2deg(log(tan(M_PI_4 + deg2rad(c.lat())/2.0))));
}

Coordinates osm::m2ll(const QPointF &p)
{
	return Coordinates(p.x(), rad2deg(2.0 * atan(exp(deg2rad(p.y()))) - M_PI_2));
}

QPoint osm::mercator2tile(const QPointF &m, int z)
{
	return QPoint((int)(floor((m.x() + 180.0) / 360.0 * (1<<z))),
	  (int)(floor((1.0 - (m.y() / 180.0)) / 2.0 * (1<<z))));
}

qreal osm::zoom2scale(int zoom, int tileSize)
{
	return (360.0/(qreal)((1<<zoom) * tileSize));
}

int osm::scale2zoom(qreal scale, int tileSize)
{
	return (int)(log2(360.0/(scale * (qreal)tileSize)) + EPSILON);
}
