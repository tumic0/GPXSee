#include "data/data.h"
#include "cadencegraphitem.h"
#include "cadencegraph.h"


CadenceGraph::CadenceGraph(QWidget *parent) : GraphTab(parent)
{
	_showTracks = true;

	GraphView::setYUnits(tr("1/min"));
	setYLabel(tr("Cadence"));

	setSliderPrecision(1);
}

void CadenceGraph::setInfo()
{
	if (_showTracks) {
		GraphView::addInfo(tr("Average"), QString::number(avg() * yScale()
		  + yOffset(), 'f', 1) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Maximum"), QString::number(max() * yScale()
		  + yOffset(),  'f', 1) + UNIT_SPACE + yUnits());
	} else
		clearInfo();
}

void CadenceGraph::loadData(const Data &data, const QList<PathItem *> &paths)
{
	for (int i = 0; i < data.tracks().count(); i++) {
		const Graph &graph = data.tracks().at(i)->cadence();

		if (graph.size() < 2) {
			skipColor();
			continue;
		}

		CadenceGraphItem *gi = new CadenceGraphItem(graph, _graphType);
		GraphView::addGraph(gi, paths.at(i));

		_avg.append(QPointF(data.tracks().at(i)->distance(), gi->avg()));
	}

	for (int i = 0; i < data.routes().count(); i++)
		skipColor();

	setInfo();

	redraw();
}

qreal CadenceGraph::avg() const
{
	qreal sum = 0, w = 0;
	QList<QPointF>::const_iterator it;

	for (it = _avg.begin(); it != _avg.end(); it++) {
		sum += it->y() * it->x();
		w += it->x();
	}

	return (sum / w);
}

void CadenceGraph::clear()
{
	_avg.clear();

	GraphView::clear();
}

void CadenceGraph::showTracks(bool show)
{
	_showTracks = show;

	showGraph(show);
	setInfo();

	redraw();
}
