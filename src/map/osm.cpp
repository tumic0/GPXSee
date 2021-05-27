#include <QtCore>
#include "common/wgs84.h"
#include "osm.h"


#define EPSILON       1e-6

static double index2mercator(int index, int zoom)
{
	return rad2deg(-M_PI + 2 * M_PI * ((double)index / (1<<zoom)));
}

QPointF OSM::ll2m(const Coordinates &c)
{
	return QPointF(c.lon(), rad2deg(log(tan(M_PI_4 + deg2rad(c.lat())/2.0))));
}

Coordinates OSM::m2ll(const QPointF &p)
{
	return Coordinates(p.x(), rad2deg(2.0 * atan(exp(deg2rad(p.y()))) - M_PI_2));
}

QPoint OSM::mercator2tile(const QPointF &m, int zoom)
{
	return QPoint(qMin(qFloor((m.x() + 180.0) / 360.0 * (1<<zoom)), (1<<zoom) - 1),
	  qMin(qFloor((1.0 - (m.y() / 180.0)) / 2.0 * (1<<zoom)), (1<<zoom) - 1));
}

QPointF OSM::tile2mercator(const QPoint &p, int zoom)
{
	return QPointF(index2mercator(p.x(), zoom), index2mercator(p.y(), zoom));
}

qreal OSM::zoom2scale(int zoom, int tileSize)
{
	return (360.0/(qreal)((1<<zoom) * tileSize));
}

int OSM::scale2zoom(qreal scale, int tileSize)
{
	return (int)(log2(360.0/(scale * (qreal)tileSize)) + EPSILON);
}

qreal OSM::resolution(const QPointF &p, int zoom, int tileSize)
{
	qreal scale = zoom2scale(zoom, tileSize);

	return (WGS84_RADIUS * 2.0 * M_PI * scale / 360.0
	  * cos(2.0 * atan(exp(deg2rad(-p.y() * scale))) - M_PI/2));
}
