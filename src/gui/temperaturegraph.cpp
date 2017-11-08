#include "data.h"
#include "temperaturegraphitem.h"
#include "temperaturegraph.h"


TemperatureGraph::TemperatureGraph(GeoItems &geoItems, QWidget *parent) : GraphTab(geoItems, parent)
{
	_showTracks = true;

	setYUnits(Metric);
	setYLabel(tr("Temperature"));

	setSliderPrecision(1);
}

void TemperatureGraph::setInfo()
{
	if (_showTracks) {
		GraphView::addInfo(tr("Average"), QString::number(avg() * yScale()
		  + yOffset(), 'f', 1) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Minimum"), QString::number(min() * yScale()
		  + yOffset(),  'f', 1) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Maximum"), QString::number(max() * yScale()
		  + yOffset(),  'f', 1) + UNIT_SPACE + yUnits());
	} else
		clearInfo();
}

qreal TemperatureGraph::avg() const
{
	qreal sum = 0, w = 0;
	QList<QPointF>::const_iterator it;

	for (it = _avg.begin(); it != _avg.end(); it++) {
		sum += it->y() * it->x();
		w += it->x();
	}

	return (sum / w);
}

void TemperatureGraph::clear()
{
	_avg.clear();

	GraphView::clear();
}

void TemperatureGraph::setYUnits(Units units)
{
	if (units == Metric) {
		GraphView::setYUnits(QChar(0x00B0) + tr("C"));
		setYScale(1);
		setYOffset(0);
	} else {
		GraphView::setYUnits(QChar(0x00B0) + tr("F"));
		setYScale(C2FS);
		setYOffset(C2FO);
	}
}

void TemperatureGraph::setUnits(Units units)
{
	setYUnits(units);
	setInfo();

	GraphView::setUnits(units);
}

void TemperatureGraph::showTracks(bool show)
{
	_showTracks = show;

	showGraph(show);
	setInfo();

	redraw();
}

void TemperatureGraph::addTrack(const Track &track, TrackItem *item)
{
	const Graph &graph = track.temperature();

	if (graph.size() >= 2) {
		TemperatureGraphItem *gi = new TemperatureGraphItem(graph, _graphType);
		GraphView::addGraph(gi, item);

		_avg.append(QPointF(track.distance(), gi->avg()));

		setInfo();
		redraw();
	}
}
