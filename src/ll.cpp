#include <cmath>
#include "ll.h"

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif // M_PI

#define WGS84_RADIUS 6378137.0
#define deg2rad(d) (((d)*M_PI)/180.0)
#define rad2deg(d) (((d)*180.0)/M_PI)

qreal llDistance(const QPointF &p1, const QPointF &p2)
{
	qreal dLat = deg2rad(p2.y() - p1.y());
	qreal dLon = deg2rad(p2.x() - p1.x());
	qreal a = pow(sin(dLat / 2.0), 2.0)
	  + cos(deg2rad(p1.y())) * cos(deg2rad(p2.y())) * pow(sin(dLon / 2.0), 2.0);
	qreal c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));

	return (WGS84_RADIUS * c);
}

QPointF ll2mercator(const QPointF &ll)
{
	QPointF m;

	m.setX(ll.x());
	m.setY(rad2deg(log(tan(M_PI/4.0 + deg2rad(ll.y())/2.0))));

	return m;
}

QPoint mercator2tile(const QPointF &m, int z)
{
	QPoint tile;

	tile.setX((int)(floor((m.x() + 180.0) / 360.0 * pow(2.0, z))));
	tile.setY((int)(floor((1.0 - (m.y() / 180.0)) / 2.0 * pow(2.0, z))));

	return tile;
}

QPointF tile2mercator(const QPoint &tile, int z)
{
	QPointF m;

	m.setX(tile.x() / pow(2.0, z) * 360.0 - 180);
	qreal n = M_PI - 2.0 * M_PI * tile.y() / pow(2.0, z);
	m.setY(rad2deg(atan(0.5 * (exp(n) - exp(-n)))));

	return ll2mercator(m);
}

int scale2zoom(qreal scale)
{
	return (int)log2(360.0/(scale * (qreal)TILE_SIZE));
}
