#ifndef ELEVATIONGRAPH_H
#define ELEVATIONGRAPH_H

#include "graph.h"

class ElevationGraph : public Graph
{
	Q_OBJECT

public:
	ElevationGraph(QWidget *parent = 0);

	void loadData(const QVector<QPointF> &data);
	void clear();

private:
	qreal _ascent, _descent;
	qreal _max, _min;
};

#endif // ELEVATIONGRAPH_H
