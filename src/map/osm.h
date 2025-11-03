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

	inline QPointF ll2m(const Coordinates &c)
	{
		return QPointF(c.lon(),
		  rad2deg(log(tan(M_PI_4 + deg2rad(c.lat())/2.0))));
	}
	inline Coordinates m2ll(const QPointF &p)
	{
		return Coordinates(p.x(),
		  rad2deg(2.0 * atan(exp(deg2rad(p.y()))) - M_PI_2));
	}
	QPoint mercator2tile(const QPointF &m, int zoom);
	QPointF tile2mercator(const QPoint &p, int zoom);
	inline qreal zoom2scale(int zoom, int tileSize)
	  {return (360.0/(qreal)((1<<zoom) * tileSize));}
	int scale2zoom(qreal scale, int tileSize);
	qreal resolution(const QPointF &p, int zoom, int tileSize);

	inline Coordinates tile2ll(const QPoint &p, int zoom)
	  {return m2ll(tile2mercator(p, zoom));}
	inline QPoint ll2tile(const Coordinates &c, int zoom)
	  {return mercator2tile(ll2m(c), zoom);}
}

#endif // OSM_H
