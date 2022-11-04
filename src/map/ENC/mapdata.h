#ifndef ENC_MAPDATA_H
#define ENC_MAPDATA_H

#include <climits>
#include <QPainterPath>
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
		Poly(uint type, const Polygon &path)
		  : _type(type), _path(path) {}

		RectC bounds() const {return _path.boundingRect();}
		const Polygon &path() const {return _path;}
		uint type() const {return _type;}

	private:
		uint _type;
		Polygon _path;
	};

	class Line {
	public:
		Line(uint type, const QVector<Coordinates> &path, const QString &label)
		  : _type(type), _path(path), _label(label) {}

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
		Point(uint type, const Coordinates &c, const QString &label)
		  : _type(type), _pos(c), _label(label)
		{
			uint hash = (uint)qHash(QPair<double,double>(c.lon(), c.lat()));
			_id = ((quint64)type)<<32 | hash;
		}

		const Coordinates &pos() const {return _pos;}
		uint type() const {return _type;}
		const QString &label() const {return _label;}

		bool operator<(const Point &other) const
		  {return _id < other._id;}

	private:
		uint _type;
		Coordinates _pos;
		QString _label;
		quint64 _id;
	};

	MapData(const QString &path);
	~MapData();

	const QString &name() const {return _name;}
	RectC bounds() const;

	void polygons(const RectC &rect, QList<Poly*> *polygons);
	void lines(const RectC &rect, QList<Line*> *lines);
	void points(const RectC &rect, QList<Point*> *points);

	void load();
	void clear();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

private:
	class Rect {
	public:
		Rect()
		  : _minX(INT_MAX), _maxX(INT_MIN), _minY(INT_MAX), _maxY(INT_MIN) {}
		Rect(int minX, int maxX, int minY, int maxY)
		  : _minX(minX), _maxX(maxX), _minY(minY), _maxY(maxY) {}

		int minX() const {return _minX;}
		int maxX() const {return _maxX;}
		int minY() const {return _minY;}
		int maxY() const {return _maxY;}

		void unite(int x, int y) {
			if (x < _minX)
				_minX = x;
			if (x > _maxX)
				_maxX = x;
			if (y < _minY)
				_minY = y;
			if (y > _maxY)
				_maxY = y;
		}

		Rect &operator|=(const Rect &r) {*this = *this | r; return *this;}
		Rect operator|(const Rect &r) const
		{
			return Rect(qMin(_minX, r._minX), qMax(_maxX, r._maxX),
			  qMin(_minY, r._minY), qMax(_maxY, r._maxY));
		}

	private:
		int _minX, _maxX, _minY, _maxY;
	};

	class Attr {
	public:
		Attr() : _subtype(0) {}
		Attr(uint subtype, const QString &label = QString())
			: _subtype(subtype), _label(label) {}

		unsigned subtype() const {return _subtype;}
		const QString &label() const {return _label;}

	private:
		unsigned _subtype;
		QString _label;
	};

	struct Sounding {
		Sounding(const Coordinates &c, double depth) : c(c), depth(depth) {}

		Coordinates c;
		double depth;
	};

	typedef QMap<uint, ISO8211::Record> RecordMap;
	typedef QMap<uint, ISO8211::Record>::const_iterator RecordMapIterator;
	typedef RTree<Poly*, double, 2> PolygonTree;
	typedef RTree<Line*, double, 2> LineTree;
	typedef RTree<Point*, double, 2> PointTree;

	bool processRecord(const ISO8211::Record &record);
	Attr pointAttr(const ISO8211::Record &r, uint OBJL);
	Attr lineAttr(const ISO8211::Record &r, uint OBJL);
	Attr polyAttr(const ISO8211::Record &r, uint OBJL);
	QVector<Sounding> soundingGeometry(const ISO8211::Record &r);
	Coordinates pointGeometry(const ISO8211::Record &r);
	QVector<Coordinates> lineGeometry(const ISO8211::Record &r);
	Polygon polyGeometry(const ISO8211::Record &r);
	Point *pointObject(const Sounding &s);
	Point *pointObject(const ISO8211::Record &r, uint OBJL);
	Line *lineObject(const ISO8211::Record &r, uint OBJL);
	Poly *polyObject(const ISO8211::Record &r, uint OBJL);
	Coordinates coordinates(int x, int y) const;
	Coordinates point(const ISO8211::Record &r);
	QVector<Sounding> soundings(const ISO8211::Record &r);

	static bool bounds(const ISO8211::Record &record, Rect &rect);
	static Rect bounds(const RecordMap &map);

	QString _name;
	RecordMap _vi, _vc, _ve, _vf;
	QVector<ISO8211::Record> _fe;
	uint _COMF, _SOMF;
	ISO8211 _ddf;

	PolygonTree _areas;
	LineTree _lines;
	PointTree _points;

	bool _valid;
	QString _errorString;
};

}

#endif // ENC_MAPDATA_H
