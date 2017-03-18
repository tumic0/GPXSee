#include "path.h"

QRectF Path::boundingRect() const
{
	if (size() < 2)
		return QRectF();

	QPointF topLeft(at(0).coordinates().lon(), at(0).coordinates().lat());
	QPointF bottomRight(topLeft);

	for (int i = 1; i < size(); i++) {
		qreal x = at(i).coordinates().lon();
		qreal y = at(i).coordinates().lat();

		if (x < topLeft.x())
			topLeft.setX(x);
		if (y < topLeft.y())
			topLeft.setY(y);
		if (x > bottomRight.x())
			bottomRight.setX(x);
		if (y > bottomRight.y())
			bottomRight.setY(y);
	}
	return QRectF(topLeft, bottomRight);
}

QDebug operator<<(QDebug dbg, const PathPoint &point)
{
	dbg.nospace() << "PathPoint(" << point.distance() << ", "
	  << point.coordinates() << ")";

	return dbg.maybeSpace();
}
