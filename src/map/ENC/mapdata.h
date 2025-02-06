#ifndef ENC_MAPDATA_H
#define ENC_MAPDATA_H

#include "common/rectc.h"
#include "common/rtree.h"
#include "common/polygon.h"
#include "iso8211.h"

namespace ENC {

class MapData
{
public:
	typedef QMap<uint, QByteArray> Attributes;

	class Poly {
	public:
		Poly(uint type, const Polygon &path, const Attributes &attr, uint HUNI);

		RectC bounds() const {return _path.boundingRect();}
		const Polygon &path() const {return _path;}
		uint type() const {return _type;}
		const Attributes &attributes() const {return _attr;}
		uint HUNI() const {return _HUNI;}

	private:
		uint _type;
		Polygon _path;
		Attributes _attr;
		uint _HUNI;
	};

	class Line {
	public:
		Line(uint type, const QVector<Coordinates> &path, const Attributes &attr);

		RectC bounds() const;
		const QVector<Coordinates> &path() const {return _path;}
		uint type() const {return _type;}
		const QString &label() const {return _label;}
		const Attributes &attributes() const {return _attr;}

	private:
		uint _type;
		QVector<Coordinates> _path;
		QString _label;
		Attributes _attr;
	};

	class Point {
	public:
		Point(uint type, const Coordinates &c, const Attributes &attr, uint HUNI);
		Point(uint type, const Coordinates &s, const QString &label);

		const Coordinates &pos() const {return _pos;}
		uint type() const {return _type;}
		const QString &label() const {return _label;}
		const Attributes &attributes() const {return _attr;}

		bool operator<(const Point &other) const
		  {return _id < other._id;}

	private:
		uint _type;
		Coordinates _pos;
		QString _label;
		quint64 _id;
		Attributes _attr;
	};

	MapData(const QString &path);
	~MapData();

	void polygons(const RectC &rect, QList<Poly> *polygons) const;
	void lines(const RectC &rect, QList<Line> *lines) const;
	void points(const RectC &rect, QList<Point> *points) const;

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
