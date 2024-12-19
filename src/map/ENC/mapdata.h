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
	class Poly {
	public:
		Poly(uint type, const Polygon &path, const QString &label,
		  const QVector<QByteArray> &params, uint HUNI);

		RectC bounds() const {return _path.boundingRect();}
		const Polygon &path() const {return _path;}
		uint type() const {return _type;}
		const QString &label() const {return _label;}
		const QVariant &param() const {return _param;}

	private:
		uint _type;
		Polygon _path;
		QString _label;
		QVariant _param;
	};

	class Line {
	public:
		Line(uint type, const QVector<Coordinates> &path, const QString &label,
		  const QVector<QByteArray> &params);

		RectC bounds() const;
		const QVector<Coordinates> &path() const {return _path;}
		uint type() const {return _type;}
		const QString &label() const {return _label;}

	private:
		uint _type;
		QVector<Coordinates> _path;
		QString _label;
	};

	class Point {
	public:
		Point(uint type, const Coordinates &c, const QString &label,
		  const QVector<QByteArray> &params);
		Point(uint type, const Coordinates &c, const QString &label,
		  const QVariant &param);

		const Coordinates &pos() const {return _pos;}
		uint type() const {return _type;}
		const QString &label() const {return _label;}
		const QVariant &param() const {return _param;}

		bool operator<(const Point &other) const
		  {return _id < other._id;}

	private:
		uint _type;
		Coordinates _pos;
		QString _label;
		quint64 _id;
		QVariant _param;
	};

	MapData(const QString &path);
	~MapData();

	void polygons(const RectC &rect, QList<Poly> *polygons) const;
	void lines(const RectC &rect, QList<Line> *lines) const;
	void points(const RectC &rect, QList<Point> *points) const;

private:
	class Attr {
	public:
		Attr() : _subtype(0) {}
		Attr(uint subtype, const QString &label,
		  const QVector<QByteArray> &params)
		  : _subtype(subtype), _label(label), _params(params) {}

		unsigned subtype() const {return _subtype;}
		const QString &label() const {return _label;}
		const QVector<QByteArray> &params() const {return _params;}

	private:
		unsigned _subtype;
		QString _label;
		QVector<QByteArray> _params;
	};

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
	static Attr pointAttr(const ISO8211::Record &r, uint OBJL);
	static Attr lineAttr(const ISO8211::Record &r, uint OBJL);
	static Attr polyAttr(const ISO8211::Record &r, uint OBJL);
	static Point *pointObject(const Sounding &s);
	static Point *pointObject(const ISO8211::Record &r, const RecordMap &vi,
	  const RecordMap &vc, uint COMF, uint OBJL);
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
