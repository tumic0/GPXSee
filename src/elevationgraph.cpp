#include <float.h>
#include "config.h"
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
	Graph::setXScale(M2KM);
}

void ElevationGraph::addInfo()
{
	Graph::addInfo(tr("Ascent"), QString::number(_ascent * _yScale, 'f', 0)
	  + THIN_SPACE + _yUnits);
	Graph::addInfo(tr("Descent"), QString::number(_descent * _yScale, 'f', 0)
	  + THIN_SPACE + _yUnits);
	Graph::addInfo(tr("Maximum"), QString::number(_max * _yScale, 'f', 0)
	  + THIN_SPACE + _yUnits);
	Graph::addInfo(tr("Minimum"), QString::number(_min * _yScale, 'f', 0)
	  + THIN_SPACE + _yUnits);
}

void ElevationGraph::loadGPX(const GPX &gpx)
{
	for (int i = 0; i < gpx.count(); i++) {
		QVector<QPointF> data;
		qreal min, max, ascent = 0, descent = 0;

		gpx.elevationGraph(i, data);
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

		addInfo();
		loadData(data);
	}
}

void ElevationGraph::clear()
{
	_ascent = 0;
	_descent = 0;
	_max = -FLT_MAX;
	_min = FLT_MAX;

	Graph::clear();
}

void ElevationGraph::setUnits(enum Units units)
{
	if (units == Metric) {
		Graph::setXUnits(tr("km"));
		Graph::setYUnits(tr("m"));
		Graph::setXScale(M2KM);
		Graph::setYScale(1);
	} else {
		Graph::setXUnits(tr("mi"));
		Graph::setYUnits(tr("ft"));
		Graph::setXScale(M2MI);
		Graph::setYScale(M2FT);
	}

	clearInfo();
	addInfo();

	redraw();
}
