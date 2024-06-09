#ifndef ENC_ATLASDATA_H
#define ENC_ATLASDATA_H

#include <QCache>
#include <QMutex>
#include "common/rtree.h"
#include "mapdata.h"

namespace ENC {

typedef QCache<QString, MapData> MapCache;

class AtlasData
{
public:
	AtlasData(MapCache &cache, QMutex &cacheLock)
	  : _cache(cache), _cacheLock(cacheLock) {}
	~AtlasData();

	void addMap(const RectC &bounds, const QString &path);

	void polys(const RectC &rect, QList<MapData::Poly> *polygons,
	  QList<MapData::Line> *lines);
	void points(const RectC &rect, QList<MapData::Point> *points);

private:
	struct MapEntry {
		MapEntry(const QString &path) : path(path) {}

		QString path;
		QMutex lock;
	};

	typedef RTree<MapEntry*, double, 2> MapTree;

	struct PolyCTX
	{
		PolyCTX(const RectC &rect,  QList<MapData::Poly> *polygons,
		  QList<MapData::Line> *lines, MapCache &cache, QMutex &cacheLock)
		  : rect(rect), polygons(polygons), lines(lines), cache(cache),
		  cacheLock(cacheLock) {}

		const RectC &rect;
		QList<MapData::Poly> *polygons;
		QList<MapData::Line> *lines;
		MapCache &cache;
		QMutex &cacheLock;
	};

	struct PointCTX
	{
		PointCTX(const RectC &rect, QList<MapData::Point> *points,
		  MapCache &cache, QMutex &cacheLock)
		  : rect(rect), points(points), cache(cache), cacheLock(cacheLock) {}

		const RectC &rect;
		QList<MapData::Point> *points;
		MapCache &cache;
		QMutex &cacheLock;
	};

	static bool polyCb(MapEntry *map, void *context);
	static bool pointCb(MapEntry *map, void *context);

	MapTree _tree;
	MapCache &_cache;
	QMutex &_cacheLock;
};

}

#endif // ENC_ATLASDATA_H
