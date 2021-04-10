#ifndef AREA_H
#define AREA_H

#include <QString>
#include <QList>
#include "common/polygon.h"

class Area
{
public:
	Area() {}
	Area(const RectC &rect)
	{
		Polygon polygon(rect);
		_polygons.reserve(1);
		_polygons.append(polygon);
		_boundingRect = polygon.boundingRect();
	}
	Area(const Polygon &polygon)
	{
		_polygons.reserve(1);
		_polygons.append(polygon);
		_boundingRect = polygon.boundingRect();
	}

	const QString &name() const {return _name;}
	const QString &description() const {return _desc;}
	const QList<Polygon> &polygons() const {return _polygons;}
	const RectC &boundingRect() const {return _boundingRect;}

	bool isValid() const
	{
		if (_polygons.isEmpty())
			return false;
		for (int i = 0; i < _polygons.size(); i++)
			if (_polygons.at(i).isEmpty() || _polygons.at(i).first().size() < 3)
				return false;

		return true;
	}

	void append(const Polygon &polygon)
	{
		_polygons.append(polygon);
		_boundingRect |= polygon.boundingRect();
	}

	void setName(const QString &name) {_name = name;}
	void setDescription(const QString &desc) {_desc = desc;}

private:
	QList<Polygon> _polygons;
	QString _name;
	QString _desc;
	RectC _boundingRect;
};

#endif // AREA_H
