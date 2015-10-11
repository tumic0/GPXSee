#include "speedgraph.h"

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
	qreal max = 0, sum = 0, avg, distance, w;

	distance = (data.at(data.size() - 1).x() - data.at(0).x());

	for (int i = 1; i < data.size(); i++) {
		qreal val = data.at(i).y();

		sum += val;

		if (val > max)
			max = val;
	}
	avg = sum / data.size();

	_avg.append(QPointF(avg, distance));

	sum = 0; w = 0;
	for (QList<QPointF>::iterator it = _avg.begin(); it != _avg.end(); it++) {
		sum += it->x() * it->y();
		w += it->y();
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
