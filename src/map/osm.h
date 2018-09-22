#ifndef OSM_H
#define OSM_H

#include <QPointF>
#include <common/coordinates.h>
#include <common/rectc.h>
#include <common/range.h>

namespace osm
{
	static const RectC bounds(Coordinates(-180, 85.0511),
	  Coordinates(180, -85.0511));
	static const Range zooms(0, 19);

	QPointF ll2m(const Coordinates &c);
	Coordinates m2ll(const QPointF &p);
	QPoint mercator2tile(const QPointF &m, int z);
	qreal zoom2scale(int zoom, int tileSize);
	int scale2zoom(qreal scale, int tileSize);
}

#endif // OSM_H
