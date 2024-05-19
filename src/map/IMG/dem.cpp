#include "dem.h"

using namespace IMG;

#define DELTA 1e-9 /* ensure col+1/row+1 is in the next tile */

static double interpolate(double dx, double dy, double p0, double p1, double p2,
  double p3)
{
	return p0 * (1.0 - dx) * (1.0 - dy) + p1 * dx * (1.0 - dy)
	  + p2 * dy * (1.0 - dx) + p3 * dx * dy;
}

static double val(const Matrix<qint16> &m, int row, int col)
{
	qint16 v = m.at(row, col);
	return (v == -32768) ? NAN : (double)v;
}

bool DEM::edgeCb(const MapData::Elevation *e, void *context)
{
	EdgeCTX *ctx = (EdgeCTX*)context;

	double x = (ctx->c.lon() - e->rect.left()) / e->xr;
	double y = (e->rect.top() - ctx->c.lat()) / e->yr;
	int row = qMin((int)y, e->m.h() - 1);
	int col = qMin((int)x, e->m.w() - 1);

	ctx->ele = val(e->m, row, col);

	return std::isnan(ctx->ele);
}

double DEM::edge(const DEMTRee &tree, const Coordinates &c)
{
	double min[2], max[2];
	double ele = NAN;
	EdgeCTX ctx(c, ele);

	min[0] = c.lon();
	min[1] = c.lat();
	max[0] = c.lon();
	max[1] = c.lat();

	tree.Search(min, max, edgeCb, &ctx);

	return ele;
}

double DEM::elevation(const DEMTRee &tree, const MapData::Elevation *e,
  const Coordinates &c)
{
	double x = (c.lon() - e->rect.left()) / e->xr;
	double y = (e->rect.top() - c.lat()) / e->yr;
	int row = qMin((int)y, e->m.h() - 1);
	int col = qMin((int)x, e->m.w() - 1);

	double p0 = val(e->m, row, col);
	double p1 = (col == e->m.w() - 1)
	  ? edge(tree, Coordinates(e->rect.left() + (col + 1) * e->xr + DELTA,
		e->rect.top() - row * e->yr))
	  : val(e->m, row, col + 1);
	double p2 = (row == e->m.h() - 1)
	  ? edge(tree, Coordinates(e->rect.left() + col * e->xr,
		e->rect.top() - (row + 1) * e->yr - DELTA))
	  : val(e->m, row + 1, col);
	double p3 = ((col == e->m.w() - 1) || (row == e->m.h() - 1))
	  ? edge(tree, Coordinates(e->rect.left() + (col + 1) * e->xr + DELTA,
		e->rect.top() - (row + 1) * e->yr - DELTA))
	  : val(e->m, row + 1, col + 1);

	return interpolate(x - col, y - row, p0, p1, p2, p3);
}

void DEM::buildTree(const QList<MapData::Elevation> &tiles, DEMTRee &tree)
{
	double min[2], max[2];

	for (int i = 0; i < tiles.size(); i++) {
		const MapData::Elevation &e = tiles.at(i);

		min[0] = e.rect.left();
		min[1] = e.rect.bottom();
		max[0] = e.rect.right();
		max[1] = e.rect.top();

		tree.Insert(min, max, &e);
	}
}

bool DEM::elevationCb(const MapData::Elevation *e, void *context)
{
	ElevationCTX *ctx = (ElevationCTX*)context;

	ctx->ele = elevation(ctx->tree, e, ctx->c);
	return std::isnan(ctx->ele);
}

void DEM::searchTree(const DEMTRee &tree, const Coordinates &c,
  double &ele)
{
	double min[2], max[2];
	ElevationCTX ctx(tree, c, ele);

	min[0] = c.lon();
	min[1] = c.lat();
	max[0] = c.lon();
	max[1] = c.lat();

	ele = NAN;
	tree.Search(min, max, elevationCb, &ctx);
}
