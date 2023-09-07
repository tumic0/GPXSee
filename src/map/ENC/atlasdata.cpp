#include "atlasdata.h"

using namespace ENC;

bool AtlasData::pointCb(const QString *map, void *context)
{
	PointCTX *ctx = (PointCTX*)context;

	ctx->lock.lock();

	MapData *cached = ctx->cache.object(map);

	if (!cached) {
		MapData *data = new MapData(*map);
		data->points(ctx->rect, ctx->points);
		if (!ctx->cache.insert(map, data))
			delete data;
	} else
		cached->points(ctx->rect, ctx->points);

	ctx->lock.unlock();

	return true;
}

bool AtlasData::polyCb(const QString *map, void *context)
{
	PolyCTX *ctx = (PolyCTX*)context;

	ctx->lock.lock();

	MapData *cached = ctx->cache.object(map);

	if (!cached) {
		MapData *data = new MapData(*map);
		data->polygons(ctx->rect, ctx->polygons);
		data->lines(ctx->rect, ctx->lines);
		if (!ctx->cache.insert(map, data))
			delete data;
	} else {
		cached->polygons(ctx->rect, ctx->polygons);
		cached->lines(ctx->rect, ctx->lines);
	}

	ctx->lock.unlock();

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

	_tree.Insert(min, max, new QString(path));
}

void AtlasData::polys(const RectC &rect, QList<MapData::Poly> *polygons,
  QList<MapData::Line> *lines)
{
	double min[2], max[2];
	PolyCTX polyCtx(rect, polygons, lines, _cache, _lock);

	min[0] = rect.left();
	min[1] = rect.bottom();
	max[0] = rect.right();
	max[1] = rect.top();

	_tree.Search(min, max, polyCb, &polyCtx);
}

void AtlasData::points(const RectC &rect, QList<MapData::Point> *points)
{
	double min[2], max[2];
	PointCTX pointCtx(rect, points, _cache, _lock);

	min[0] = rect.left();
	min[1] = rect.bottom();
	max[0] = rect.right();
	max[1] = rect.top();

	_tree.Search(min, max, pointCb, &pointCtx);
}
