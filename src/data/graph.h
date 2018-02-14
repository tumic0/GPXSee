#ifndef GRAPH_H
#define GRAPH_H

#include <QVector>
#include <QDebug>
#include <cmath>

enum GraphType {Distance, Time};

class GraphPoint
{
public:
	GraphPoint(qreal s = NAN, qreal t = NAN, qreal y = NAN)
	  : _s(s), _t(t), _y(y) {}

	qreal s() const {return _s;}
	qreal t() const {return _t;}
	qreal y() const {return _y;}
	qreal x(GraphType type) const {return (type == Distance) ? _s : _t;}

	void setS(qreal s) {_s = s;}
	void setT(qreal t) {_t = t;}
	void setY(qreal y) {_y = y;}

private:
	qreal _s;
	qreal _t;
	qreal _y;
};

Q_DECLARE_TYPEINFO(GraphPoint, Q_PRIMITIVE_TYPE);

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const GraphPoint &point)
{
	dbg.nospace() << "GraphPoint(" << point.s() << ", " << point.t() << ", "
	  << point.y() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

typedef QVector<GraphPoint> Graph;

#endif // GRAPH_H
