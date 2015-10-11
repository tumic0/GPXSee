#include <float.h>
#include "elevationgraph.h"

ElevationGraph::ElevationGraph()
{
	_ascent = 0;
	_descent = 0;
	_max = -FLT_MAX;
	_min = FLT_MAX;

	Graph::setXLabel(tr("Distance"));
	Graph::setYLabel(tr("Elevation"));
	Graph::setXUnits(tr("km"));
	Graph::setYUnits(tr("m"));
	Graph::setXScale(0.001);
}

void ElevationGraph::loadData(const QVector<QPointF> &data)
{
	qreal ascent = 0, descent = 0;
	qreal min = data.at(0).y();
	qreal max = data.at(0).y();


	for (int i = 1; i < data.size(); i++) {
		qreal cur = data.at(i).y();
		qreal prev = data.at(i-1).y();

		if (cur > prev)
			ascent += cur - prev;
		if (cur < prev)
			descent += prev - cur;

		if (cur > max)
			max = cur;
		if (cur < min)
			min = cur;
	}

	_ascent += ascent;
	_descent += descent;
	_max = qMax(_max, max);
	_min = qMin(_min, min);

	addInfo(tr("Ascent"), QString::number((int)_ascent) + " " + _yUnits);
	addInfo(tr("Descent"), QString::number((int)_descent) + " " + _yUnits);
	addInfo(tr("Minimum"), QString::number((int)_min) + " " + _yUnits);
	addInfo(tr("Maximum"), QString::number((int)_max) + " " + _yUnits);

	Graph::loadData(data);
}

void ElevationGraph::clear()
{
	_ascent = 0;
	_descent = 0;
	_max = -FLT_MAX;
	_min = FLT_MAX;

	Graph::clear();
}
