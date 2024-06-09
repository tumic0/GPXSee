#include "atlasdata.h"

using namespace ENC;

bool AtlasData::pointCb(MapEntry *map, void *context)
{
	PointCTX *ctx = (PointCTX*)context;

start:
	ctx->cacheLock.lock();

	MapData *cached = ctx->cache.object(map->path);

	if (!cached) {
		ctx->cacheLock.unlock();

		if (map->lock.tryLock()) {
			MapData *data = new MapData(map->path);
			data->points(ctx->rect, ctx->points);

			ctx->cacheLock.lock();
			ctx->cache.insert(map->path, data);

			map->lock.unlock();
		} else {
			map->lock.lock();
			map->lock.unlock();
			goto start;
		}
	} else
		cached->points(ctx->rect, ctx->points);

	ctx->cacheLock.unlock();

	return true;
}

bool AtlasData::polyCb(MapEntry *map, void *context)
{
	PolyCTX *ctx = (PolyCTX*)context;

start:
	ctx->cacheLock.lock();

	MapData *cached = ctx->cache.object(map->path);

	if (!cached) {
		ctx->cacheLock.unlock();

		if (map->lock.tryLock()) {
			MapData *data = new MapData(map->path);
			data->polygons(ctx->rect, ctx->polygons);
			data->lines(ctx->rect, ctx->lines);

			ctx->cacheLock.lock();
			ctx->cache.insert(map->path, data);

			map->lock.unlock();
		} else {
			map->lock.lock();
			map->lock.unlock();
			goto start;
		}
	} else {
		cached->polygons(ctx->rect, ctx->polygons);
		cached->lines(ctx->rect, ctx->lines);
	}

	ctx->cacheLock.unlock();

	return true;
}

AtlasData::~AtlasData()
{
	MapTree::Iterator it;
	for (_tree.GetFirst(it); !_tree.IsNull(it); _tree.GetNext(it))
		delete _tree.GetAt(it);
}

void AtlasData::addMap(const RectC &bounds, const QString &path)
{
	double min[2], max[2];

	min[0] = bounds.left();
	min[1] = bounds.bottom();
	max[0] = bounds.right();
	max[1] = bounds.top();

	_tree.Insert(min, max, new MapEntry(path));
}

void AtlasData::polys(const RectC &rect, QList<MapData::Poly> *polygons,
  QList<MapData::Line> *lines)
{
	double min[2], max[2];
	PolyCTX polyCtx(rect, polygons, lines, _cache, _cacheLock);

	min[0] = rect.left();
	min[1] = rect.bottom();
	max[0] = rect.right();
	max[1] = rect.top();

	_tree.Search(min, max, polyCb, &polyCtx);
}

void AtlasData::points(const RectC &rect, QList<MapData::Point> *points)
{
	double min[2], max[2];
	PointCTX pointCtx(rect, points, _cache, _cacheLock);

	min[0] = rect.left();
	min[1] = rect.bottom();
	max[0] = rect.right();
	max[1] = rect.top();

	_tree.Search(min, max, pointCb, &pointCtx);
}
