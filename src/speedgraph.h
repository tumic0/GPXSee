#ifndef SPEEDGRAPH_H
#define SPEEDGRAPH_H

#include <QList>
#include "graph.h"

class SpeedGraph : public Graph
{
	Q_OBJECT

public:
	SpeedGraph();

	void loadData(const QVector<QPointF> &data, qreal time);
	void clear();

private:
	qreal _max;
	QList<QPointF> _avg;
};

#endif // SPEEDGRAPH_H
