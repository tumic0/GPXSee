#include "config.h"
#include "gpx.h"
#include "elevationgraph.h"


ElevationGraph::ElevationGraph(QWidget *parent) : GraphView(parent)
{
	_ascent = 0;
	_descent = 0;

	setXLabel(tr("Distance"));
	setYLabel(tr("Elevation"));
	setXUnits(tr("km"));
	setYUnits(tr("m"));
	setXScale(M2KM);
	setMinYRange(50.0);
}

void ElevationGraph::addInfo()
{
	GraphView::addInfo(tr("Ascent"), QString::number(_ascent * yScale(), 'f', 0)
	  + UNIT_SPACE + yUnits());
	GraphView::addInfo(tr("Descent"), QString::number(_descent * yScale(), 'f',
	  0) + UNIT_SPACE + yUnits());
	GraphView::addInfo(tr("Maximum"), QString::number(max() * yScale(), 'f', 0)
	  + UNIT_SPACE + yUnits());
	GraphView::addInfo(tr("Minimum"), QString::number(min() * yScale(), 'f', 0)
	  + UNIT_SPACE + yUnits());

	redraw();
}

void ElevationGraph::loadGPX(const GPX &gpx)
{
	for (int i = 0; i < gpx.trackCount(); i++) {
		QVector<QPointF> data;
		qreal ascent = 0, descent = 0;

		gpx.track(i).elevationGraph(data);
		if (data.count() < 2) {
			skipColor();
			continue;
		}

		for (int j = 1; j < data.size(); j++) {
			qreal cur = data.at(j).y();
			qreal prev = data.at(j-1).y();

			if (cur > prev)
				ascent += cur - prev;
			if (cur < prev)
				descent += prev - cur;
		}

		_ascent += ascent;
		_descent += descent;

		loadData(data);
	}

	addInfo();
}

void ElevationGraph::clear()
{
	_ascent = 0;
	_descent = 0;

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
}
