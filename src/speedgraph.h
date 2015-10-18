#ifndef SPEEDGRAPH_H
#define SPEEDGRAPH_H

#include <QList>
#include "graph.h"
#include "gpx.h"

class SpeedGraph : public Graph
{
	Q_OBJECT

public:
	SpeedGraph(QWidget *parent = 0);

	void loadGPX(const GPX &gpx);
	void clear();

	qreal avg() const;
	qreal max() const {return _max;}

private:
	qreal _max;
	QList<QPointF> _avg;
};

#endif // SPEEDGRAPH_H
