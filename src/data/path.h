#ifndef PATH_H
#define PATH_H

#include <QVector>
#include <QRectF>
#include "common/coordinates.h"
#include "common/rectc.h"
#include "style.h"

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
#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const PathPoint &point);
#endif // QT_NO_DEBUG

typedef QVector<PathPoint> PathSegment;

class Path : public QList<PathSegment>
{
public:
	bool isValid() const;
	RectC boundingRect() const;

	const LineStyle &style() const {return _style;}
	void setStyle(const LineStyle &style) {_style = style;}

private:
	LineStyle _style;
};

#endif // PATH_H
