#ifndef SPEEDGRAPH_H
#define SPEEDGRAPH_H

#include <QList>
#include "graph.h"

class SpeedGraph : public Graph
{
public:
	SpeedGraph();

	void loadData(const QVector<QPointF> &data);
	void clear();

private:
	qreal _max;
	QList<QPointF> _avg;
};

#endif // SPEEDGRAPH_H
