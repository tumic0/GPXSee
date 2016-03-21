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

	setXLabel(tr("Distance"));
	setYLabel(tr("Elevation"));
	setXUnits(tr("km"));
	setYUnits(tr("m"));
	setXScale(M2KM);
	setMinRange(50.0);
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
		if (data.count() < 2) {
			skipColor();
			continue;
		}

		min = max = data.at(0).y();

		for (int j = 1; j < data.size(); j++) {
			qreal cur = data.at(j).y();
			qreal prev = data.at(j-1).y();

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
		setXUnits(tr("km"));
		setYUnits(tr("m"));
		setXScale(M2KM);
		setYScale(1);
	} else {
		setXUnits(tr("mi"));
		setYUnits(tr("ft"));
		setXScale(M2MI);
		setYScale(M2FT);
	}

	clearInfo();
	addInfo();

	redraw();
}
