#ifndef GRAPH_H
#define GRAPH_H

#include <QVector>
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

typedef QVector<GraphPoint> Graph;

#endif // GRAPH_H
