#include "common/garmin.h"
#include "jls.h"
#include "demfile.h"

using namespace IMG;
using namespace Garmin;

static qint16 limit(const DEMTile *tile, quint16 factor)
{
	quint8 f1 = (tile->flags() & 1) != 0;
	qint16 l = f1 ? tile->diff() - factor : tile->diff() + 1;

	if (tile->flags() > 1) {
		for (int i = 1; i < 8; i++) {
			if (((tile->flags() >> i) & 1) != 0)
				l = (l - 1) - (factor << f1);
		}
	}

	return l;
}

void DEMFile::clear()
{
	_levels.clear();
}

bool DEMFile::load(Handle &hdl)
{
	quint32 u32, zoomData;
	quint16 zooms, zoomDataSize;

	if (!(seek(hdl, _gmpOffset + 0x15) && readUInt32(hdl, _flags)
	  && readUInt16(hdl, zooms) && readUInt32(hdl, u32)
	  && readUInt16(hdl, zoomDataSize) && readUInt32(hdl, zoomData)))
		return false;

	_levels.reserve(zooms);

	for (quint16 i = 0; i < zooms; i++) {
		quint32 pixelWidth, pixelHeight, pixelWidth2, pixelHeight2, table, cols,
		  rows, xr, yr, data;
		qint32 lon, lat;
		quint16 encoding, size, factor;
		qint16 minHeight, maxHeight;
		quint8 layer, level;
		QList<DEMTile> tiles;

		if (!(seek(hdl, zoomData + i * zoomDataSize) && readUInt8(hdl, layer)
		  && readUInt8(hdl, level) && readUInt32(hdl, pixelHeight)
		  && readUInt32(hdl, pixelWidth) && readUInt32(hdl, pixelHeight2)
		  && readUInt32(hdl, pixelWidth2) && readUInt16(hdl, factor)
		  && readUInt32(hdl, cols) && readUInt32(hdl, rows)
		  && readUInt16(hdl, encoding) && readUInt16(hdl, size)
		  && readUInt32(hdl, table) && readUInt32(hdl, data)
		  && readInt32(hdl, lon) && readInt32(hdl, lat)
		  && readUInt32(hdl, yr) && readUInt32(hdl, xr)
		  && readInt16(hdl, minHeight) && readInt16(hdl, maxHeight)))
			return false;

		if (layer)
			continue;

		if (!seek(hdl, table))
			return false;

		tiles.reserve((rows + 1) * (cols + 1));

		for (quint32 i = 0; i < rows + 1; i++) {
			for (quint32 j = 0; j < cols + 1; j++) {
				qint32 x = lon + j * pixelWidth * xr;
				qint32 y = lat - i * pixelHeight * yr;
				quint32 w = (j == cols) ? (pixelWidth2 + 1) : pixelWidth;
				quint32 h = (i == rows) ? (pixelHeight2 + 1) : pixelHeight;
				RectC r(Coordinates(toWGS32(x), toWGS32(y)),
				  Coordinates(toWGS32(x + w * xr), toWGS32(y - h * yr)));

				quint32 offset;
				qint16 base;
				quint16 diff;
				quint8 flags = 0;

				if (!readVUInt32(hdl, (encoding & 0x3) + 1, offset))
					return false;
				if (encoding & 0x4) {
					if (!readInt16(hdl, base))
						return false;
				} else {
					if (!readInt8(hdl, base))
						return false;
				}
				if (encoding & 0x8) {
					if (!readUInt16(hdl, diff))
						return false;
				} else {
					if (!readUInt8(hdl, diff))
						return false;
				}
				if ((encoding & 0x10) && !readUInt8(hdl, flags))
					return false;

				tiles.append(DEMTile(r, w, h, offset, base, diff, flags));
			}
		}

		_levels.append(Level(RectC(tiles.first().rect().topLeft(),
		  tiles.last().rect().bottomRight()), toWGS32(xr), toWGS32(yr),
		  toWGS32(pixelWidth * xr), toWGS32(pixelHeight* yr),
		  data, rows + 1, cols + 1, factor, level, minHeight, maxHeight, tiles));
	}

	return !_levels.isEmpty();
}

QList<const DEMTile*> DEMFile::tiles(const RectC &rect, int level) const
{
	const Level &lvl = _levels.at(level);
	QList<const DEMTile*> ret;

	RectC ir(lvl.rect & rect);
	double left = (ir.left() - lvl.rect.left()) / lvl.txr;
	double top = (lvl.rect.top() - ir.top()) / lvl.tyr;
	double right = (ir.right() - lvl.rect.left()) / lvl.txr;
	double bottom = (lvl.rect.top() - ir.bottom()) / lvl.tyr;
	quint32 t = qMin((quint32)top, lvl.rows - 1);
	quint32 l = qMin((quint32)left, lvl.cols - 1);
	quint32 b = qMin((quint32)bottom, lvl.rows - 1);
	quint32 r = qMin((quint32)right, lvl.cols - 1);

	ret.reserve((b - t + 1) * (r - l + 1));

	for (quint32 i = t; i <= b; i++)
		for (quint32 j = l; j <= r; j++)
			ret.append(&lvl.tiles.at(lvl.cols * i + j));

	return ret;
}

int DEMFile::level(const Zoom &zoom) const
{
/*
	for (int i = 0; i < _levels.size(); i++)
		if (_levels.at(i).level >= zoom.level())
			return i;

	return _levels.size() - 1;
*/

	Q_UNUSED(zoom);
	return 0;
}

MapData::Elevation *DEMFile::elevations(Handle &hdl, int level,
  const DEMTile *tile)
{
	const Level &l = _levels.at(level);
	MapData::Elevation *ele = new MapData::Elevation();
	ele->rect = tile->rect();
	ele->xr = l.xr;
	ele->yr = l.yr;

	if (!tile->diff()) {
		ele->m = Matrix<qint16>(tile->h(), tile->w(),
		  tile->flags() ? -32768 : meters(tile->base()));
		return ele;
	}

	if (!seek(hdl, tile->offset() + l.data))
		return ele;

	quint16 lim = limit(tile, l.factor);
	Matrix<qint16> m(tile->h(), tile->w());
	JLS jls(tile->diff(), l.factor, tile->w());
	if (jls.decode(this, hdl, m)) {
		for (int i = 0; i < m.size(); i++) {
			if (m.at(i) >= lim)
				m.at(i) = -32768;
			else {
				m.at(i) += tile->base();
				if (m.at(i) < l.minHeight)
					m.at(i) = l.minHeight;
				if (m.at(i) > l.maxHeight)
					m.at(i) = l.maxHeight;
				m.at(i) = meters(m.at(i));
			}
		}

		ele->m = m;
	}

	return ele;
}
