#ifndef GRAPH_H
#define GRAPH_H

#include <QList>
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

typedef QVector<GraphPoint> GraphSegment;

class Graph : public QList<GraphSegment>
{
public:
	bool isValid() const
	{
		if (isEmpty())
			return false;
		for (int i = 0; i < size(); i++)
			if (at(i).size() < 2)
				return false;
		return true;
	}

	bool hasTime() const
	{
		for (int i = 0; i < size(); i++) {
			const GraphSegment &segment = at(i);
			for (int j = 0; j < segment.size(); j++)
				if (std::isnan(segment.at(j).t()))
					return false;
		}
		return true;
	}
};

class GraphPair
{
public:
	GraphPair(const Graph &primary, const Graph &secondary)
	  : _primary(primary), _secondary(secondary) {}

	const Graph &primary() const {return _primary;}
	const Graph &secondary() const {return _secondary;}

private:
	Graph _primary, _secondary;
};

#endif // GRAPH_H
