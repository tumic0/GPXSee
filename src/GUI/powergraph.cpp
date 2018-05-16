#include "data/data.h"
#include "powergraphitem.h"
#include "powergraph.h"


PowerGraph::PowerGraph(QWidget *parent) : GraphTab(parent)
{
	_showTracks = true;

	GraphView::setYUnits(tr("W"));
	setYLabel(tr("Power"));

	setSliderPrecision(1);
}

void PowerGraph::setInfo()
{
	if (_showTracks) {
		GraphView::addInfo(tr("Average"), QString::number(avg() * yScale()
		  + yOffset(), 'f', 1) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Maximum"), QString::number(max() * yScale()
		  + yOffset(),  'f', 1) + UNIT_SPACE + yUnits());
	} else
		clearInfo();
}

QList<GraphItem*> PowerGraph::loadData(const Data &data)
{
	QList<GraphItem*> graphs;

	for (int i = 0; i < data.tracks().count(); i++) {
		const Graph &graph = data.tracks().at(i)->power();

		if (graph.size() < 2) {
			skipColor();
			graphs.append(0);
		} else {
			PowerGraphItem *gi = new PowerGraphItem(graph, _graphType);
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

qreal PowerGraph::avg() const
{
	qreal sum = 0, w = 0;
	QList<QPointF>::const_iterator it;

	for (it = _avg.begin(); it != _avg.end(); it++) {
		sum += it->y() * it->x();
		w += it->x();
	}

	return (sum / w);
}

void PowerGraph::clear()
{
	_avg.clear();

	GraphView::clear();
}

void PowerGraph::showTracks(bool show)
{
	_showTracks = show;

	showGraph(show);
	setInfo();

	redraw();
}
