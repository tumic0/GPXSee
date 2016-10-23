#include "data.h"
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

void TemperatureGraph::loadData(const Data &data, const QList<PathItem *> &paths)
{
	for (int i = 0; i < data.tracks().count(); i++) {
		const Graph &graph = data.tracks().at(i)->temperature();
		qreal sum = 0, w = 0;

		if (graph.size() < 2) {
			skipColor();
			continue;
		}

		for (int j = 1; j < graph.size(); j++) {
			qreal ds = graph.at(j).s() - graph.at(j-1).s();
			sum += graph.at(j).y() * ds;
			w += ds;
		}
		_avg.append(QPointF(data.tracks().at(i)->distance(), sum/w));

		GraphView::loadGraph(graph, paths.at(i));
	}

	for (int i = 0; i < data.routes().count(); i++)
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
