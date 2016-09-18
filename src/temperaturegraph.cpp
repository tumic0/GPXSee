#include "gpx.h"
#include "temperaturegraph.h"


TemperatureGraph::TemperatureGraph(QWidget *parent) : GraphTab(parent)
{
	_units = Metric;
	_showTracks = true;

	setYUnits();
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

void TemperatureGraph::loadGPX(const GPX &gpx, const QList<PathItem *> &paths)
{
	for (int i = 0; i < gpx.tracks().count(); i++) {
		const Graph &graph = gpx.tracks().at(i)->temperature();
		qreal sum = 0, w = 0;

		if (graph.y.count() < 2) {
			skipColor();
			continue;
		}

		for (int j = 1; j < graph.y.size(); j++) {
			qreal ds = graph.distance.at(j) - graph.distance.at(j-1);
			sum += graph.y.at(j) * ds;
			w += ds;
		}
		_avg.append(QPointF(gpx.tracks().at(i)->distance(), sum/w));

		GraphView::loadGraph(graph, paths.at(i));
	}

	for (int i = 0; i < gpx.routes().count(); i++)
		skipColor();

	setInfo();

	redraw();
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

void TemperatureGraph::setYUnits()
{
	if (_units == Metric) {
		GraphView::setYUnits(QChar(0x00B0) + tr("C"));
		setYScale(1);
		setYOffset(0);
	} else {
		GraphView::setYUnits(QChar(0x00B0) + tr("F"));
		setYScale(C2FS);
		setYOffset(C2FO);
	}
}

void TemperatureGraph::setUnits(enum Units units)
{
	_units = units;

	setYUnits();
	setInfo();
	GraphView::setUnits(units);

	redraw();
}

void TemperatureGraph::showTracks(bool show)
{
	_showTracks = show;

	showGraph(show);
	setInfo();

	redraw();
}
