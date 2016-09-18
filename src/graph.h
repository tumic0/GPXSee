#ifndef GRAPH_H
#define GRAPH_H

#include <QVector>

class Graph
{
public:
	enum Type {Distance, Time};

	QVector<qreal> distance;
	QVector<qreal> time;
	QVector<qreal> y;
};

#endif // GRAPH_H
