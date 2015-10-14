#include "speedgraph.h"

#include <QDebug>

SpeedGraph::SpeedGraph()
{
	_max = 0;

	Graph::setXLabel(tr("Distance"));
	Graph::setYLabel(tr("Speed"));
	Graph::setXUnits(tr("km"));
	Graph::setYUnits(tr("km/h"));
	Graph::setXScale(0.001);
	Graph::setYScale(3.6);
}

void SpeedGraph::loadData(const QVector<QPointF> &data)
{
	qreal max = 0, sum = 0, w = 0, avg;
	qreal dist;

	if (data.size() < 2)
		return;

	dist = data.at(data.size() - 1).x() - data.at(0).x();


	for (int i = 1; i < data.size(); i++) {
		QPointF cur = data.at(i);
		QPointF prev = data.at(i-1);
		qreal ds = cur.x() - prev.x();

		if (cur.y() == 0)
			continue;
		sum += ds;
		w += ds / cur.y();

		max = qMax(max, cur.y());
	}
	avg = sum / w;

	_avg.append(QPointF(dist, avg));

	sum = 0; w = 0;
	for (QList<QPointF>::iterator it = _avg.begin(); it != _avg.end(); it++) {
		sum += it->y() * it->x();
		w += it->x();
	}
	avg = sum / w;

	_max = qMax(_max, max);

	addInfo(tr("Average"), QString::number(avg * _yScale, 'f', 1)
	  + " " + _yUnits);
	addInfo(tr("Maximum"), QString::number(_max * _yScale,  'f', 1)
	  + " " + _yUnits);

	Graph::loadData(data);
}

void SpeedGraph::clear()
{
	_max = 0;
	_avg.clear();

	Graph::clear();
}
