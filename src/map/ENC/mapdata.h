#ifndef ENC_MAPDATA_H
#define ENC_MAPDATA_H

#include "common/rtree.h"
#include "iso8211.h"
#include "data.h"

namespace ENC {

class MapData : public Data
{
public:
	MapData(const QString &path);
	virtual ~MapData();

	virtual void polys(const RectC &rect, QList<Poly> *polygons,
	  QList<Line> *lines);
	virtual void points(const RectC &rect, QList<Point> *points);

private:
	struct Sounding {
		Sounding() : depth(NAN) {}
		Sounding(const Coordinates &c, double depth) : c(c), depth(depth) {}

		Coordinates c;
		double depth;
	};

	typedef QMap<uint, ISO8211::Record> RecordMap;
	typedef QMap<uint, ISO8211::Record>::const_iterator RecordMapIterator;
	typedef RTree<const Poly*, double, 2> PolygonTree;
	typedef RTree<const Line*, double, 2> LineTree;
	typedef RTree<const Point*, double, 2> PointTree;

	static QVector<Sounding> soundings(const ISO8211::Record &r, uint COMF,
	  uint SOMF);
	static QVector<Sounding> soundingGeometry(const ISO8211::Record &r,
	  const RecordMap &vi, const RecordMap &vc, uint COMF, uint SOMF);
	static Coordinates pointGeometry(const ISO8211::Record &r,
	  const RecordMap &vi, const RecordMap &vc, uint COMF);
	static QVector<Coordinates> lineGeometry(const ISO8211::Record &r,
	  const RecordMap &vc, const RecordMap &ve, uint COMF);
	static Polygon polyGeometry(const ISO8211::Record &r, const RecordMap &vc,
	  const RecordMap &ve, uint COMF);
	static Attributes attributes(const ISO8211::Record &r);
	static Point *pointObject(const Sounding &s);
	static Point *pointObject(const ISO8211::Record &r, const RecordMap &vi,
	  const RecordMap &vc, uint COMF, uint OBJL, uint HUNI);
	static Line *lineObject(const ISO8211::Record &r, const RecordMap &vc,
	  const RecordMap &ve, uint COMF, uint OBJL);
	static Poly *polyObject(const ISO8211::Record &r, const RecordMap &vc,
	  const RecordMap &ve, uint COMF, uint OBJL, uint HUNI);

	static bool processRecord(const ISO8211::Record &record,
	  QVector<ISO8211::Record> &fe, RecordMap &vi, RecordMap &vc, RecordMap &ve,
	  RecordMap &vf, uint &COMF, uint &SOMF, uint &HUNI);

	PolygonTree _areas;
	LineTree _lines;
	PointTree _points;
};

}

#endif // ENC_MAPDATA_H
