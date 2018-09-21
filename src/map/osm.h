#ifndef OSM_H
#define OSM_H

#include <QPointF>
#include <common/coordinates.h>

namespace osm
{
	QPointF ll2m(const Coordinates &c);
	Coordinates m2ll(const QPointF &p);
	QPoint mercator2tile(const QPointF &m, int z);
	qreal zoom2scale(int zoom, int tileSize);
	int scale2zoom(qreal scale, int tileSize);
}

#endif // OSM_H
