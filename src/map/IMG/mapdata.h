#ifndef IMG_MAPDATA_H
#define IMG_MAPDATA_H

#include <QList>
#include <QPointF>
#include <QCache>
#include <QMutex>
#include <QDebug>
#include "common/rectc.h"
#include "common/rtree.h"
#include "common/range.h"
#include "common/hash.h"
#include "label.h"
#include "raster.h"
#include "zoom.h"

namespace IMG {

class Style;
class SubDiv;
class SubFile;
class VectorTile;

class MapData
{
public:
	struct Poly {
		Poly() : oneway(false) {}

		/* QPointF insted of Coordinates for performance reasons (no need to
		   duplicate all the vectors for drawing). Note, that we do not want to
		   ll2xy() the points in the MapData class as this can not be done in
		   parallel. */
		QVector<QPointF> points;
		Label label;
		Raster raster;
		quint32 type;
		RectC boundingRect;
		bool oneway;

		bool operator<(const Poly &other) const
		  {return type > other.type;}
	};

	struct Point {
		Point() : id(0), classLabel(false) {}

		Coordinates coordinates;
		Label label;
		quint32 type;
		quint64 id;
		bool classLabel;

		bool operator<(const Point &other) const
		  {return id < other.id;}
	};

	MapData(const QString &fileName);
	virtual ~MapData();

	const QString &name() const {return _name;}
	const RectC &bounds() const {return _bounds;}
	const Range &zooms() const {return _zoomLevels;}
	const Style *style() const {return _style;}
	void polys(const RectC &rect, int bits, QList<Poly> *polygons,
	  QList<Poly> *lines);
	void points(const RectC &rect, int bits, QList<Point> *points);

	void load(qreal ratio);
	void clear();

	const QString &fileName() const {return _fileName;}

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

protected:
	typedef RTree<VectorTile*, double, 2> TileTree;

	void computeZooms();

	QString _fileName;
	QString _name;
	RectC _bounds;
	SubFile *_typ;
	Style *_style;
	TileTree _tileTree;
	QList<Zoom> _zooms;
	Range _zoomLevels;

	bool _valid;
	QString _errorString;

private:
	struct Polys {
		Polys() {}
		Polys(const QList<Poly> &polygons, const QList<Poly> &lines)
			: polygons(polygons), lines(lines) {}

		QList<Poly> polygons;
		QList<Poly> lines;
	};

	typedef QCache<const SubDiv*, Polys> PolyCache;
	typedef QCache<const SubDiv*, QList<Point> > PointCache;

	struct PolyCTX
	{
		PolyCTX(const RectC &rect, const Zoom &zoom,
		  QList<MapData::Poly> *polygons, QList<MapData::Poly> *lines,
		  PolyCache *polyCache, QMutex *lock)
		  : rect(rect), zoom(zoom), polygons(polygons), lines(lines),
		  polyCache(polyCache), lock(lock) {}

		const RectC &rect;
		const Zoom &zoom;
		QList<MapData::Poly> *polygons;
		QList<MapData::Poly> *lines;
		PolyCache *polyCache;
		QMutex *lock;
	};

	struct PointCTX
	{
		PointCTX(const RectC &rect, const Zoom &zoom,
		  QList<MapData::Point> *points, PointCache *pointCache, QMutex *lock)
		  : rect(rect), zoom(zoom), points(points), pointCache(pointCache),
		  lock(lock) {}

		const RectC &rect;
		const Zoom &zoom;
		QList<MapData::Point> *points;
		PointCache *pointCache;
		QMutex *lock;
	};

	const Zoom &zoom(int bits) const;

	static bool polyCb(VectorTile *tile, void *context);
	static bool pointCb(VectorTile *tile, void *context);

	PolyCache _polyCache;
	PointCache _pointCache;
	QMutex _lock;

	friend class VectorTile;
};

}

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const IMG::MapData::Point &point)
{
	dbg.nospace() << "Point(" << point.type << ", " << point.label << ")";
	return dbg.space();
}

inline QDebug operator<<(QDebug dbg, const IMG::MapData::Poly &poly)
{
	dbg.nospace() << "Poly(" << poly.type << ", " << poly.label << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // IMG_MAPDATA_H
