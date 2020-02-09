#include "common/programpaths.h"
#include "vectortile.h"
#include "style.h"
#include "mapdata.h"


struct PolyCTX
{
	PolyCTX(const RectC &rect, int bits, QList<MapData::Poly> *polygons,
	  QList<MapData::Poly> *lines, QCache<const SubDiv*,
	  MapData::Polys> *polyCache) : rect(rect), bits(bits), polygons(polygons),
	  lines(lines), polyCache(polyCache) {}

	const RectC &rect;
	int bits;
	QList<MapData::Poly> *polygons;
	QList<MapData::Poly> *lines;
	QCache<const SubDiv*, MapData::Polys> *polyCache;
};

struct PointCTX
{
	PointCTX(const RectC &rect, int bits, QList<MapData::Point> *points,
	  QCache<const SubDiv*, QList<MapData::Point> > *pointCache)
	  : rect(rect), bits(bits), points(points), pointCache(pointCache) {}

	const RectC &rect;
	int bits;
	QList<MapData::Point> *points;
	QCache<const SubDiv*, QList<MapData::Point> > *pointCache;
};

inline bool polyCb(VectorTile *tile, void *context)
{
	PolyCTX *ctx = (PolyCTX*)context;
	tile->polys(ctx->rect, ctx->bits, ctx->polygons, ctx->lines, ctx->polyCache);
	return true;
}

inline bool pointCb(VectorTile *tile, void *context)
{
	PointCTX *ctx = (PointCTX*)context;
	tile->points(ctx->rect, ctx->bits, ctx->points, ctx->pointCache);
	return true;
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
	PolyCTX ctx(rect, bits, polygons, lines, &_polyCache);
	double min[2], max[2];

	min[0] = rect.left();
	min[1] = rect.bottom();
	max[0] = rect.right();
	max[1] = rect.top();

	_tileTree.Search(min, max, polyCb, &ctx);
}

void MapData::points(const RectC &rect, int bits, QList<Point> *points)
{
	PointCTX ctx(rect, bits, points, &_pointCache);
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
		SubFile typ(ProgramPaths::typFile());
		_style = new Style(&typ);
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
