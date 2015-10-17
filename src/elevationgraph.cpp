#include <float.h>
#include "elevationgraph.h"

ElevationGraph::ElevationGraph(QWidget *parent) : Graph(parent)
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

void ElevationGraph::loadGPX(const GPX &gpx)
{
	QVector<QPointF> data;
	qreal min, max, ascent = 0, descent = 0;


	gpx.elevationGraph(data);
	if (data.isEmpty())
		return;

	min = max = data.at(0).y();

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

	addInfo(tr("Ascent"), QString::number(_ascent, 'f', 0) + " " + _yUnits);
	addInfo(tr("Descent"), QString::number(_descent, 'f', 0) + " " + _yUnits);
	addInfo(tr("Maximum"), QString::number(_max, 'f', 0) + " " + _yUnits);
	addInfo(tr("Minimum"), QString::number(_min, 'f', 0) + " " + _yUnits);

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
