#include <cmath>
#include "ll.h"

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif // M_PI

#define WGS84_RADIUS 6378137.0
#define deg2rad(d) (((d)*M_PI)/180.0)
#define rad2deg(d) (((d)*180.0)/M_PI)

qreal llDistance(const QPointF &p1, const QPointF &p2)
{
	qreal dLat = deg2rad(p2.y() - p1.y());
	qreal dLon = deg2rad(p2.x() - p1.x());
	qreal a = pow(sin(dLat / 2.0), 2.0)
	  + cos(deg2rad(p1.y())) * cos(deg2rad(p2.y())) * pow(sin(dLon / 2.0), 2.0);
	qreal c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));

	return (WGS84_RADIUS * c);
}

void ll2mercator(const QPointF &src, QPointF &dst)
{
	dst.setX(src.x());
	dst.setY(rad2deg(log(tan(M_PI/4.0 + deg2rad(src.y())/2.0))));
}
