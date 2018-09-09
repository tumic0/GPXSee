#include <QLocale>
#include "data/data.h"
#include "temperaturegraphitem.h"
#include "temperaturegraph.h"


TemperatureGraph::TemperatureGraph(QWidget *parent) : GraphTab(parent)
{
	_showTracks = true;

	setYUnits(Metric);
	setYLabel(tr("Temperature"));

	setSliderPrecision(1);
}

void TemperatureGraph::setInfo()
{
	if (_showTracks) {
		QLocale l(QLocale::system());

		GraphView::addInfo(tr("Average"), l.toString(avg() * yScale()
		  + yOffset(), 'f', 1) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Minimum"), l.toString(min() * yScale()
		  + yOffset(),  'f', 1) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Maximum"), l.toString(max() * yScale()
		  + yOffset(),  'f', 1) + UNIT_SPACE + yUnits());
	} else
		clearInfo();
}

QList<GraphItem*> TemperatureGraph::loadData(const Data &data)
{
	QList<GraphItem*> graphs;

	for (int i = 0; i < data.tracks().count(); i++) {
		const Graph &graph = data.tracks().at(i)->temperature();

		if (graph.size() < 2) {
			skipColor();
			graphs.append(0);
		} else {
			TemperatureGraphItem *gi = new TemperatureGraphItem(graph,
			  _graphType);
			GraphView::addGraph(gi);
			_avg.append(QPointF(data.tracks().at(i)->distance(), gi->avg()));
			graphs.append(gi);
		}
	}

	for (int i = 0; i < data.routes().count(); i++) {
		skipColor();
		graphs.append(0);
	}

	setInfo();
	redraw();

	return graphs;
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
