#include "config.h"
#include "gpx.h"
#include "elevationgraph.h"


ElevationGraph::ElevationGraph(QWidget *parent) : GraphView(parent)
{
	_ascent = 0;
	_descent = 0;

	_units = Metric;

	setYUnits();
	setXLabel(tr("Distance"));
	setYLabel(tr("Elevation"));

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

	setXUnits();
	addInfo();
}

void ElevationGraph::clear()
{
	_ascent = 0;
	_descent = 0;

	GraphView::clear();
}

void ElevationGraph::setXUnits()
{
	if (_units == Metric) {
		if (bounds().width() < KMINM) {
			GraphView::setXUnits(tr("m"));
			setXScale(1);
		} else {
			GraphView::setXUnits(tr("km"));
			setXScale(M2KM);
		}
	} else {
		if (bounds().width() < MIINM) {
			GraphView::setXUnits(tr("ft"));
			setXScale(M2FT);
		} else {
			GraphView::setXUnits(tr("mi"));
			setXScale(M2MI);
		}
	}
}

void ElevationGraph::setYUnits()
{
	if (_units == Metric) {
		GraphView::setYUnits(tr("m"));
		setYScale(1);
	} else {
		GraphView::setYUnits(tr("ft"));
		setYScale(M2FT);
	}
}

void ElevationGraph::setUnits(enum Units units)
{
	_units = units;
	setXUnits();
	setYUnits();

	clearInfo();
	addInfo();
}
