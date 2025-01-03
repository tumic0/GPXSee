#include <QFileInfo>
#include "common/programpaths.h"
#include "vectortile.h"
#include "style.h"
#include "mapdata.h"


using namespace IMG;

#define CACHED_SUBDIVS_COUNT  2048 // ~32MB for both caches together
#define CACHED_DEMTILES_COUNT 1024 // ~32MB

bool MapData::polyCb(VectorTile *tile, void *context)
{
	PolyCTX *ctx = (PolyCTX*)context;
	tile->polys(ctx->file, ctx->rect, ctx->zoom, ctx->polygons, ctx->lines,
	  ctx->cache, ctx->lock);
	return true;
}

bool MapData::pointCb(VectorTile *tile, void *context)
{
	PointCTX *ctx = (PointCTX*)context;
	tile->points(ctx->file, ctx->rect, ctx->zoom, ctx->points, ctx->cache,
	  ctx->lock);
	return true;
}

bool MapData::elevationCb(VectorTile *tile, void *context)
{
	ElevationCTX *ctx = (ElevationCTX*)context;
	tile->elevations(ctx->file, ctx->rect, ctx->zoom, ctx->elevations,
	  ctx->cache, ctx->lock);
	return true;
}

MapData::MapData(const QString &fileName)
  : _fileName(fileName), _typ(0), _style(0), _hasDEM(false), _valid(false)
{
	_polyCache.setMaxCost(CACHED_SUBDIVS_COUNT);
	_pointCache.setMaxCost(CACHED_SUBDIVS_COUNT);
	_demCache.setMaxCost(CACHED_DEMTILES_COUNT);
}

MapData::~MapData()
{
	TileTree::Iterator it;
	for (_tileTree.GetFirst(it); !_tileTree.IsNull(it); _tileTree.GetNext(it))
		delete _tileTree.GetAt(it);

	delete _typ;
	delete _style;
}

void MapData::polys(QFile *file, const RectC &rect, int bits,
  QList<Poly> *polygons, QList<Poly> *lines)
{
	PolyCTX ctx(file, rect, zoom(bits), polygons, lines, &_polyCache, &_lock);
	double min[2], max[2];

	min[0] = rect.left();
	min[1] = rect.bottom();
	max[0] = rect.right();
	max[1] = rect.top();

	_tileTree.Search(min, max, polyCb, &ctx);
}

void MapData::points(QFile *file, const RectC &rect, int bits,
  QList<Point> *points)
{
	PointCTX ctx(file, rect, zoom(bits), points, &_pointCache, &_lock);
	double min[2], max[2];

	min[0] = rect.left();
	min[1] = rect.bottom();
	max[0] = rect.right();
	max[1] = rect.top();

	_tileTree.Search(min, max, pointCb, &ctx);
}

void MapData::elevations(QFile *file, const RectC &rect, int bits,
  QList<Elevation> *elevations)
{
	ElevationCTX ctx(file, rect, zoom(bits), elevations, &_demCache, &_demLock);
	double min[2], max[2];

	min[0] = rect.left();
	min[1] = rect.bottom();
	max[0] = rect.right();
	max[1] = rect.top();

	_tileTree.Search(min, max, elevationCb, &ctx);
}

void MapData::load(qreal ratio)
{
	Q_ASSERT(!_style);

	if (_typ)
		_style = new Style(ratio, _typ);
	else {
		QString typFile(ProgramPaths::typFile());
		if (QFileInfo::exists(typFile)) {
			SubFile typ(typFile);
			_style = new Style(ratio, &typ);
		} else
			_style = new Style(ratio);
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
	_demCache.clear();
}

void MapData::computeZooms()
{
	TileTree::Iterator it;
	QSet<Zoom> zooms;

	for (_tileTree.GetFirst(it); !_tileTree.IsNull(it); _tileTree.GetNext(it)) {
		const QVector<Zoom> &z = _tileTree.GetAt(it)->zooms();
		for (int i = 0; i < z.size(); i++)
			zooms.insert(z.at(i));
	}

	if (zooms.isEmpty())
		return;

	_zooms = zooms.values();
	std::sort(_zooms.begin(), _zooms.end());

	bool baseMap = false;
	for (int i = 1; i < _zooms.size(); i++) {
		if (_zooms.at(i).level() > _zooms.at(i-1).level()) {
			baseMap = true;
			break;
		}
	}
	_zoomLevels = Range(baseMap ? _zooms.first().bits()
	  : qMax(0, _zooms.first().bits() - 2), 28);
}

const Zoom &MapData::zoom(int bits) const
{
	int id = 0;

	for (int i = 1; i < _zooms.size(); i++) {
		if (_zooms.at(i).bits() > bits)
			break;
		id++;
	}

	return _zooms.at(id);
}
