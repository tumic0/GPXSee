#ifndef IMG_DEMFILE_H
#define IMG_DEMFILE_H

#include "common/rtree.h"
#include "subfile.h"
#include "demtile.h"

namespace IMG {

class DEMFile : public SubFile
{
public:
	DEMFile(const IMGData *img) : SubFile(img) {}
	DEMFile(const QString &path) : SubFile(path) {}
	DEMFile(const SubFile *gmp, quint32 offset) : SubFile(gmp, offset) {}

	bool load(Handle &hdl);
	void clear();
	MapData::Elevation *elevations(Handle &hdl, int level,
	  const DEMTile *tile) const;

	int level(const Zoom &zoom) const;
	QList<const DEMTile *> tiles(const RectC &rect, int level) const;

private:
	struct Level {
		Level() {}
		Level(const RectC &rect, double xr, double yr, double txr, double tyr,
		  quint32 data, quint32 rows, quint32 cols, quint16 factor,
		  quint8 level, qint16 minHeight, qint16 maxHeight,
		  const QList<DEMTile> &tiles)
			: rect(rect), xr(xr), yr(yr), txr(txr), tyr(tyr), data(data),
			rows(rows), cols(cols), factor(factor), level(level),
			minHeight(minHeight), maxHeight(maxHeight), tiles(tiles) {}

		RectC rect;
		double xr, yr;
		double txr, tyr;
		quint32 data;
		quint32 rows, cols;
		quint16 factor;
		quint8 level;
		qint16 minHeight;
		qint16 maxHeight;
		QList<DEMTile> tiles;
	};

	qint16 meters(qint16 val) const
	{
		return (_flags & 1) ? (qint16)qRound(val * 0.3048) : val;
	}

	quint32 _flags;
	QVector<Level> _levels;
};

}

#endif // IMG_DEMFILE_H
