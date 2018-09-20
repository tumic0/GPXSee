#ifndef OSM_H
#define OSM_H

#include <QPointF>
#include <common/coordinates.h>

#define TILE_SIZE     256

namespace osm
{
	QPointF ll2m(const Coordinates &c);
	Coordinates m2ll(const QPointF &p);
	QPoint mercator2tile(const QPointF &m, int z);
	qreal zoom2scale(int zoom);
	int scale2zoom(qreal scale);
}

#endif // OSM_H
