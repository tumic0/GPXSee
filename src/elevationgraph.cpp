#include <float.h>
#include "config.h"
#include "gpx.h"
#include "elevationgraph.h"


ElevationGraph::ElevationGraph(QWidget *parent) : GraphView(parent)
{
	_ascent = 0;
	_descent = 0;
	_max = -FLT_MAX;
	_min = FLT_MAX;

	GraphView::setXLabel(tr("Distance"));
	GraphView::setYLabel(tr("Elevation"));
	GraphView::setXUnits(tr("km"));
	GraphView::setYUnits(tr("m"));
	GraphView::setXScale(M2KM);
}

void ElevationGraph::addInfo()
{
	GraphView::addInfo(tr("Ascent"), QString::number(_ascent * _yScale, 'f', 0)
	  + UNIT_SPACE + _yUnits);
	GraphView::addInfo(tr("Descent"), QString::number(_descent * _yScale, 'f', 0)
	  + UNIT_SPACE + _yUnits);
	GraphView::addInfo(tr("Maximum"), QString::number(_max * _yScale, 'f', 0)
	  + UNIT_SPACE + _yUnits);
	GraphView::addInfo(tr("Minimum"), QString::number(_min * _yScale, 'f', 0)
	  + UNIT_SPACE + _yUnits);
}

void ElevationGraph::loadGPX(const GPX &gpx)
{
	for (int i = 0; i < gpx.trackCount(); i++) {
		QVector<QPointF> data;
		qreal min, max, ascent = 0, descent = 0;

		gpx.track(i).elevationGraph(data);
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

	GraphView::clear();
}

void ElevationGraph::setUnits(enum Units units)
{
	if (units == Metric) {
		GraphView::setXUnits(tr("km"));
		GraphView::setYUnits(tr("m"));
		GraphView::setXScale(M2KM);
		GraphView::setYScale(1);
	} else {
		GraphView::setXUnits(tr("mi"));
		GraphView::setYUnits(tr("ft"));
		GraphView::setXScale(M2MI);
		GraphView::setYScale(M2FT);
	}

	clearInfo();
	addInfo();

	redraw();
}
