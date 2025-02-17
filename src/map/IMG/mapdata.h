#ifndef IMG_MAPDATA_H
#define IMG_MAPDATA_H

#include <QList>
#include <QPointF>
#include <QCache>
#include <QMutex>
#include <QFile>
#include <QDebug>
#include "common/rectc.h"
#include "common/rtree.h"
#include "common/range.h"
#include "common/hash.h"
#include "map/matrix.h"
#include "label.h"
#include "raster.h"
#include "light.h"
#include "zoom.h"

namespace IMG {

class Style;
class SubDiv;
class SubFile;
class VectorTile;
class DEMTile;

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
		Point() : id(0), flags(0) {}

		enum Flags {
			NoFlag = 0,
			ClassLabel = 1,
		};

		Coordinates coordinates;
		Label label;
		QVector<Light> lights;
		quint64 id;
		quint32 type;
		quint32 flags;

		bool operator<(const Point &other) const
		  {return id < other.id;}
	};

	struct Elevation {
		Matrix<qint16> m;
		RectC rect;
		double xr;
		double yr;
	};

	MapData(const QString &fileName);
	virtual ~MapData();

	const QString &name() const {return _name;}
	const RectC &bounds() const {return _bounds;}
	const Range &zooms() const {return _zoomLevels;}
	const Style *style() const {return _style;}
	void polys(QFile *file, const RectC &rect, int bits, QList<Poly> *polygons,
	  QList<Poly> *lines);
	void points(QFile *file, const RectC &rect, int bits, QList<Point> *points);
	void elevations(QFile *file, const RectC &rect, int bits,
	  QList<Elevation> *elevations);

	void load(qreal ratio);
	void clear();

	bool hasDEM() const {return _hasDEM;}

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
	bool _hasDEM;

	bool _valid;
	QString _errorString;

private:
	struct Polys {
		QList<Poly> polygons;
		QList<Poly> lines;
	};

	typedef QCache<const SubDiv*, Polys> PolyCache;
	typedef QCache<const SubDiv*, QList<Point> > PointCache;
	typedef QCache<const DEMTile*, Elevation> ElevationCache;

	struct PolyCTX
	{
		PolyCTX(QFile *file, const RectC &rect, const Zoom &zoom,
		  QList<MapData::Poly> *polygons, QList<MapData::Poly> *lines,
		  PolyCache *cache, QMutex *lock)
		  : file(file), rect(rect), zoom(zoom), polygons(polygons),
		  lines(lines), cache(cache), lock(lock) {}

		QFile *file;
		const RectC &rect;
		const Zoom &zoom;
		QList<MapData::Poly> *polygons;
		QList<MapData::Poly> *lines;
		PolyCache *cache;
		QMutex *lock;
	};

	struct PointCTX
	{
		PointCTX(QFile *file, const RectC &rect, const Zoom &zoom,
		  QList<MapData::Point> *points, PointCache *cache, QMutex *lock)
		  : file(file), rect(rect), zoom(zoom), points(points), cache(cache),
		  lock(lock) {}

		QFile *file;
		const RectC &rect;
		const Zoom &zoom;
		QList<MapData::Point> *points;
		PointCache *cache;
		QMutex *lock;
	};

	struct ElevationCTX
	{
		ElevationCTX(QFile *file, const RectC &rect, const Zoom &zoom,
		  QList<Elevation> *elevations, ElevationCache *cache, QMutex *lock)
		  : file(file), rect(rect), zoom(zoom), elevations(elevations),
		  cache(cache), lock(lock) {}

		QFile *file;
		const RectC &rect;
		const Zoom &zoom;
		QList<Elevation> *elevations;
		ElevationCache *cache;
		QMutex *lock;
	};

	const Zoom &zoom(int bits) const;

	static bool polyCb(VectorTile *tile, void *context);
	static bool pointCb(VectorTile *tile, void *context);
	static bool elevationCb(VectorTile *tile, void *context);

	PolyCache _polyCache;
	PointCache _pointCache;
	ElevationCache _demCache;
	QMutex _lock, _demLock;

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
