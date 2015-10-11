#ifndef ELEVATIONGRAPH_H
#define ELEVATIONGRAPH_H

#include "graph.h"

class ElevationGraph : public Graph
{
public:
	ElevationGraph();

	void loadData(const QVector<QPointF> &data);
	void clear();

private:
	qreal _ascent, _descent;
	qreal _max, _min;
};

#endif // ELEVATIONGRAPH_H
