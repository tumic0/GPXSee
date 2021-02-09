#ifndef OSM_H
#define OSM_H

#include <QPointF>
#include <common/coordinates.h>
#include <common/rectc.h>
#include <common/range.h>

namespace OSM
{
	static const RectC BOUNDS(Coordinates(-180, 85.0511),
	  Coordinates(180, -85.0511));
	static const Range ZOOMS(0, 19);

	QPointF ll2m(const Coordinates &c);
	Coordinates m2ll(const QPointF &p);
	QPoint mercator2tile(const QPointF &m, int zoom);
	double index2mercator(int index, int zoom);
	qreal zoom2scale(int zoom, int tileSize);
	int scale2zoom(qreal scale, int tileSize);
	qreal resolution(const QPointF &p, int zoom, int tileSize);
}

#endif // OSM_H
