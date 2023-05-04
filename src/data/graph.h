#ifndef GRAPH_H
#define GRAPH_H

#include <QList>
#include <QVector>
#include <QColor>
#include <QDateTime>
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

class GraphSegment : public QVector<GraphPoint>
{
public:
	GraphSegment(const QDateTime &start)
	  : _start(start) {}
	GraphSegment(int size, const QDateTime &start)
	  : QVector<GraphPoint>(size), _start(start) {}

	const QDateTime &start() const {return _start;}

private:
	QDateTime _start;
};

class Graph : public QList<GraphSegment>
{
public:
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

	const QColor &color() const {return _color;}
	void setColor(const QColor &color) {_color = color;}

private:
	QColor _color;
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
