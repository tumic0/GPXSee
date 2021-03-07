#ifndef POLYGON_H
#define POLYGON_H

#include <QList>
#include <QVector>
#include "common/coordinates.h"
#include "common/rectc.h"

class Polygon : public QList<QVector<Coordinates> >
{
public:
	Polygon() {}
	Polygon(const QVector<Coordinates> &c)
	{
		reserve(1);
		append(c);
	}

	bool isValid() const
	{
		return !isEmpty() && first().size() >= 3;
	}

	RectC boundingRect() const
	{
		RectC rect;

		if (isEmpty() || first().size() < 3)
			return rect;

		for (int i = 0; i < first().size(); i++)
			rect = rect.united(first().at(i));

		return rect;
	}
};

#endif // POLYGON_H
