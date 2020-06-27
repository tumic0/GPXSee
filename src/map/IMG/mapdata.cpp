#include "common/programpaths.h"
#include "vectortile.h"
#include "style.h"
#include "mapdata.h"


#define CACHED_SUBDIVS_COUNT 2048 // ~32MB for both caches together

struct PolyCTX
{
	PolyCTX(const RectC &rect, int bits, bool baseMap,
	  QList<MapData::Poly> *polygons, QList<MapData::Poly> *lines,
	  QCache<const SubDiv*, MapData::Polys> *polyCache)
	  : rect(rect), bits(bits), baseMap(baseMap), polygons(polygons),
	  lines(lines), polyCache(polyCache) {}

	const RectC &rect;
	int bits;
	bool baseMap;
	QList<MapData::Poly> *polygons;
	QList<MapData::Poly> *lines;
	QCache<const SubDiv*, MapData::Polys> *polyCache;
};

struct PointCTX
{
	PointCTX(const RectC &rect, int bits, bool baseMap,
	  QList<MapData::Point> *points,
	  QCache<const SubDiv*, QList<MapData::Point> > *pointCache)
	  : rect(rect), bits(bits), baseMap(baseMap), points(points),
	  pointCache(pointCache) {}

	const RectC &rect;
	int bits;
	bool baseMap;
	QList<MapData::Point> *points;
	QCache<const SubDiv*, QList<MapData::Point> > *pointCache;
};

inline bool polyCb(VectorTile *tile, void *context)
{
	PolyCTX *ctx = (PolyCTX*)context;
	tile->polys(ctx->rect, ctx->bits, ctx->baseMap, ctx->polygons, ctx->lines,
	  ctx->polyCache);
	return true;
}

inline bool pointCb(VectorTile *tile, void *context)
{
	PointCTX *ctx = (PointCTX*)context;
	tile->points(ctx->rect, ctx->bits, ctx->baseMap, ctx->points,
	  ctx->pointCache);
	return true;
}


MapData::MapData() : _typ(0), _style(0), _zooms(24, 28), _baseMap(false),
  _valid(false)
{
	_polyCache.setMaxCost(CACHED_SUBDIVS_COUNT);
	_pointCache.setMaxCost(CACHED_SUBDIVS_COUNT);
}

MapData::~MapData()
{
	TileTree::Iterator it;
	for (_tileTree.GetFirst(it); !_tileTree.IsNull(it); _tileTree.GetNext(it))
		delete _tileTree.GetAt(it);

	delete _typ;
	delete _style;
}

void MapData::polys(const RectC &rect, int bits, QList<Poly> *polygons,
  QList<Poly> *lines)
{
	PolyCTX ctx(rect, bits, _baseMap, polygons, lines, &_polyCache);
	double min[2], max[2];

	min[0] = rect.left();
	min[1] = rect.bottom();
	max[0] = rect.right();
	max[1] = rect.top();

	_tileTree.Search(min, max, polyCb, &ctx);
}

void MapData::points(const RectC &rect, int bits, QList<Point> *points)
{
	PointCTX ctx(rect, bits, _baseMap, points, &_pointCache);
	double min[2], max[2];

	min[0] = rect.left();
	min[1] = rect.bottom();
	max[0] = rect.right();
	max[1] = rect.top();

	_tileTree.Search(min, max, pointCb, &ctx);
}

void MapData::load()
{
	Q_ASSERT(!_style);

	if (_typ)
		_style = new Style(_typ);
	else {
		QString typFile(ProgramPaths::typFile());
		if (!typFile.isEmpty()) {
			SubFile typ(typFile);
			_style = new Style(&typ);
		} else
			_style = new Style();
	}
}

void MapData::clear()
{
	TileTree::Iterator it;
	for (_tileTree.GetFirst(it); !_tileTree.IsNull(it); _tileTree.GetNext(it))
		_tileTree.GetAt(it)->clear();

	delete _style;
	_style = 0;

	_polyCache.clear();
	_pointCache.clear();
}
