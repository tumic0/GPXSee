#ifndef POLYGON_H
#define POLYGON_H

#include <QList>
#include <QVector>
#include <QDebug>
#include "common/rectc.h"

class Polygon
{
public:
	Polygon() {}
	Polygon(const QVector<Coordinates> &path)
	{
		_paths.reserve(1);
		_paths.append(path);
		_boundingRect = boundingRect(path);
	}
	Polygon(const RectC &rect)
	{
		QVector<Coordinates> v(4);

		v[0] = Coordinates(rect.left(), rect.top());
		v[1] = Coordinates(rect.right(), rect.top());
		v[2] = Coordinates(rect.right(), rect.bottom());
		v[3] = Coordinates(rect.left(), rect.bottom());

		_paths.reserve(1);
		_paths.append(v);
		_boundingRect = RectC(v.at(0), v.at(2));
	}

	void append(const QVector<Coordinates> &path)
	{
		_paths.append(path);
		_boundingRect |= boundingRect(path);
	}
	void reserve(int size) {_paths.reserve(size);}

	int size() const {return _paths.size();}
	bool isEmpty() const {return _paths.isEmpty();}
	const QVector<Coordinates> &at(int i) const {return _paths.at(i);}
	const QVector<Coordinates> &first() const {return _paths.first();}
	const QVector<Coordinates> &last() const {return _paths.last();}

	const RectC &boundingRect() const {return _boundingRect;}

private:
	static RectC boundingRect(const QVector<Coordinates> &path)
	{
		RectC rect;

		for (int i = 0; i < path.size(); i++)
			rect = rect.united(path.at(i));

		return rect;
	}

	friend QDebug operator<<(QDebug dbg, const Polygon &poly);

	QList<QVector<Coordinates> > _paths;
	RectC _boundingRect;
};


#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const Polygon &poly)
{
	dbg.nospace() << "Polygon(" << poly._paths << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // POLYGON_H
