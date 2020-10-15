#ifndef MAPDATA_H
#define MAPDATA_H

#include <QList>
#include <QPointF>
#include <QCache>
#include <QDebug>
#include "common/rectc.h"
#include "common/rtree.h"
#include "common/range.h"
#include "label.h"

class Style;
class SubDiv;
class SubFile;
class VectorTile;

class MapData
{
public:
	struct Poly {
		/* QPointF insted of Coordinates for performance reasons (no need to
		   duplicate all the vectors for drawing). Note, that we do not want to
		   ll2xy() the points in the IMG class as this can not be done in
		   parallel. */
		QVector<QPointF> points;
		Label label;
		quint32 type;
		RectC boundingRect;

		bool operator<(const Poly &other) const
		  {return type > other.type;}
	};

	struct Point {
		Point() : id(0) {}

		Coordinates coordinates;
		Label label;
		quint32 type;
		quint64 id;

		bool operator<(const Point &other) const
		  {return id < other.id;}
	};

	struct Polys {
		Polys() {}
		Polys(const QList<Poly> &polygons, const QList<Poly> &lines)
			: polygons(polygons), lines(lines) {}

		QList<Poly> polygons;
		QList<Poly> lines;
	};

	MapData();
	virtual ~MapData();

	const QString &name() const {return _name;}
	const RectC &bounds() const {return _bounds;}
	const Range &zooms() const {return _zooms;}
	const Style *style() const {return _style;}
	void polys(const RectC &rect, int bits, QList<Poly> *polygons,
	  QList<Poly> *lines);
	void points(const RectC &rect, int bits, QList<Point> *points);

	void load();
	void clear();

	virtual QString fileName() const = 0;

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

protected:
	typedef RTree<VectorTile*, double, 2> TileTree;

	QString _name;
	RectC _bounds;
	SubFile *_typ;
	Style *_style;
	TileTree _tileTree;
	Range _zooms;
	bool _baseMap;

	bool _valid;
	QString _errorString;

private:
	QCache<const SubDiv*, Polys> _polyCache;
	QCache<const SubDiv*, QList<Point> > _pointCache;
};

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const MapData::Point &point)
{
	dbg.nospace() << "Point(" << hex << point.type << ", " << point.label << ")";
	return dbg.space();
}

inline QDebug operator<<(QDebug dbg, const MapData::Poly &poly)
{
	dbg.nospace() << "Poly(" << hex << poly.type << ", " << poly.label << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // MAPDATA_H
