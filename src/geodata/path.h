#ifndef PATH_H
#define PATH_H

#include <QVector>
#include <QRectF>
#include "coordinates.h"
#include "rectc.h"

class PathPoint
{
public:
	PathPoint() :
	  _coordinates(Coordinates()), _distance(NAN) {}
	PathPoint(const Coordinates &coordinates, qreal distance)
	  : _coordinates(coordinates), _distance(distance) {}

	const Coordinates &coordinates() const {return _coordinates;}
	qreal distance() const {return _distance;}

private:
	Coordinates _coordinates;
	qreal _distance;
};

Q_DECLARE_TYPEINFO(PathPoint, Q_PRIMITIVE_TYPE);
QDebug operator<<(QDebug dbg, const PathPoint &point);


class Path : public QVector<PathPoint>
{
public:
	RectC boundingRect() const;
};

#endif // PATH_H
