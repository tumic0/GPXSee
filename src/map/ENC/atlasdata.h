#ifndef ENC_ATLASDATA_H
#define ENC_ATLASDATA_H

#include <QCache>
#include <QMutex>
#include "common/rtree.h"
#include "mapdata.h"

namespace ENC {

typedef QCache<const QString*, MapData> MapCache;

class AtlasData
{
public:
	AtlasData(MapCache &cache, QMutex &lock)
	  : _cache(cache), _lock(lock) {}
	~AtlasData();

	void addMap(const RectC &bounds, const QString &path);

	void polys(const RectC &rect, QList<MapData::Poly> *polygons,
	  QList<MapData::Line> *lines);
	void points(const RectC &rect, QList<MapData::Point> *points);

private:
	typedef RTree<const QString*, double, 2> MapTree;

	struct PolyCTX
	{
		PolyCTX(const RectC &rect,  QList<MapData::Poly> *polygons,
		  QList<MapData::Line> *lines, MapCache &cache, QMutex &lock)
		  : rect(rect), polygons(polygons), lines(lines), cache(cache),
		  lock(lock) {}

		const RectC &rect;
		QList<MapData::Poly> *polygons;
		QList<MapData::Line> *lines;
		MapCache &cache;
		QMutex &lock;
	};

	struct PointCTX
	{
		PointCTX(const RectC &rect, QList<MapData::Point> *points,
		  MapCache &cache, QMutex &lock) : rect(rect), points(points),
		  cache(cache), lock(lock) {}

		const RectC &rect;
		QList<MapData::Point> *points;
		MapCache &cache;
		QMutex &lock;
	};

	static bool polyCb(const QString *map, void *context);
	static bool pointCb(const QString *map, void *context);

	MapTree _tree;
	MapCache &_cache;
	QMutex &_lock;
};

}

#endif // ENC_ATLASDATA_H
