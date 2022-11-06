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

		RectC bounds() const
		{
			RectC b;

			for (int i = 0; i < _path.size(); i++)
				b = b.united(_path.at(i));

			return b;
		}
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
	RectC bounds() const {return _bounds;}

	void polygons(const RectC &rect, QList<Poly*> *polygons);
	void lines(const RectC &rect, QList<Line*> *lines);
	void points(const RectC &rect, QList<Point*> *points);

	void load();
	void clear();

	bool isValid() const {return _bounds.isValid();}
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
		Sounding() : depth(NAN) {}
		Sounding(const Coordinates &c, double depth) : c(c), depth(depth) {}

		Coordinates c;
		double depth;
	};

	typedef QMap<uint, ISO8211::Record> RecordMap;
	typedef QMap<uint, ISO8211::Record>::const_iterator RecordMapIterator;
	typedef RTree<Poly*, double, 2> PolygonTree;
	typedef RTree<Line*, double, 2> LineTree;
	typedef RTree<Point*, double, 2> PointTree;

	bool fetchBoundsAndName();

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
	  const RecordMap &ve, uint COMF,uint OBJL);
	static bool bounds(const ISO8211::Record &record, Rect &rect);
	static bool bounds(const QVector<ISO8211::Record> &gv, Rect &b);
	static bool processRecord(const ISO8211::Record &record,
	  QVector<ISO8211::Record> &fe, RecordMap &vi, RecordMap &vc, RecordMap &ve,
	  RecordMap &vf, uint &COMF, uint &SOMF);
	static bool processRecord(const ISO8211::Record &record,
	  QVector<ISO8211::Record> &rv, uint &COMF, QString &name);

	QString _fileName;
	QString _name;
	RectC _bounds;
	PolygonTree _areas;
	LineTree _lines;
	PointTree _points;
	QString _errorString;
};

}

#endif // ENC_MAPDATA_H
