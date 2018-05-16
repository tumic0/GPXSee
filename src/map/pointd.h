#ifndef POINT_H
#define POINT_H

#include <cmath>
#include <QPointF>
#include <QDebug>

class PointD
{
public:
	PointD() : _x(NAN), _y(NAN) {}
	PointD(double x, double y) : _x(x), _y(y) {}
	PointD(const QPointF &p) : _x(p.x()), _y(p.y()) {}

	double x() const {return _x;}
	double y() const {return _y;}
	double &rx() {return _x;}
	double &ry() {return _y;}

	bool isNull() const {return std::isnan(_x) && std::isnan(_y);}

	QPointF toPointF() const {return QPointF((qreal)_x, (qreal)_y);}

private:
	double _x, _y;
};

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const PointD &p)
{
	dbg.nospace() << "PointD(" << p.x() << ", " << p.y() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // POINT_H
