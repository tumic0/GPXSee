#ifndef ENC_DATA_H
#define ENC_DATA_H

#include "common/rectc.h"
#include "common/polygon.h"

namespace ENC {

class Data
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
		Point(uint type, const Coordinates &c, const Attributes &attr,
		  uint HUNI, bool polygon = false);
		Point(uint type, const Coordinates &s, const QString &label);

		const Coordinates &pos() const {return _pos;}
		uint type() const {return _type;}
		const QString &label() const {return _label;}
		const Attributes &attributes() const {return _attr;}
		bool polygon() const {return _polygon;}

		bool operator<(const Point &other) const
		  {return _id < other._id;}

	private:
		uint _type;
		Coordinates _pos;
		QString _label;
		quint64 _id;
		Attributes _attr;
		bool _polygon;
	};

	virtual void polys(const RectC &rect, QList<Data::Poly> *polygons,
	  QList<Data::Line> *lines) = 0;
	virtual void points(const RectC &rect, QList<Data::Point> *points) = 0;
};

}

#endif // ENC_DATA_H
