#ifndef AREA_H
#define AREA_H

#include <QString>
#include <QList>
#include "polygon.h"

class Area : public QList<Polygon>
{
public:
	Area() {}
	Area(const RectC &rect)
	{
		Polygon polygon;
		QVector<Coordinates> v(4);
		v[0] = Coordinates(rect.left(), rect.top());
		v[1] = Coordinates(rect.right(), rect.top());
		v[2] = Coordinates(rect.right(), rect.bottom());
		v[3] = Coordinates(rect.left(), rect.bottom());
		polygon.append(v);
		append(polygon);
	}

	const QString& name() const {return _name;}
	const QString& description() const {return _desc;}
	void setName(const QString &name) {_name = name;}
	void setDescription(const QString &desc) {_desc = desc;}

	bool isValid() const
	{
		if (isEmpty())
			return false;
		for (int i = 0; i < size(); i++)
			if (!at(i).isValid())
				return false;
		return true;
	}

	RectC boundingRect() const
	{
		RectC ret;
		for (int i = 0; i < size(); i++)
			ret |= at(i).boundingRect();
		return ret;
	}

private:
	QString _name;
	QString _desc;
};

#endif // AREA_H
