#include <QLocale>
#include "data/data.h"
#include "cadencegraphitem.h"
#include "cadencegraph.h"


CadenceGraph::CadenceGraph(QWidget *parent) : GraphTab(parent)
{
	_showTracks = true;

	GraphView::setYUnits(tr("rpm"));
	setYLabel(tr("Cadence"));

	setSliderPrecision(1);
}

void CadenceGraph::setInfo()
{
	if (_showTracks) {
		QLocale l(QLocale::system());

		GraphView::addInfo(tr("Average"), l.toString(avg() * yScale()
		  + yOffset(), 'f', 1) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Maximum"), l.toString(max() * yScale()
		  + yOffset(),  'f', 1) + UNIT_SPACE + yUnits());
	} else
		clearInfo();
}

QList<GraphItem*> CadenceGraph::loadData(const Data &data)
{
	QList<GraphItem*> graphs;

	for (int i = 0; i < data.tracks().count(); i++) {
		const Track &track = data.tracks().at(i);
		const Graph &graph = track.cadence();

		if (!graph.isValid()) {
			skipColor();
			graphs.append(0);
		} else {
			CadenceGraphItem *gi = new CadenceGraphItem(graph, _graphType);
			GraphView::addGraph(gi);
			_avg.append(QPointF(track.distance(), gi->avg()));
			graphs.append(gi);
		}
	}

	for (int i = 0; i < data.routes().count(); i++) {
		skipColor();
		graphs.append(0);
	}

	for (int i = 0; i < data.areas().count(); i++)
		skipColor();

	setInfo();
	redraw();

	return graphs;
}

qreal CadenceGraph::avg() const
{
	qreal sum = 0, w = 0;

	for (int i = 0; i < _avg.size(); i++) {
		const QPointF &p = _avg.at(i);
		sum += p.y() * p.x();
		w += p.x();
	}

	return (sum / w);
}

void CadenceGraph::clear()
{
	_avg.clear();

	GraphTab::clear();
}

void CadenceGraph::showTracks(bool show)
{
	_showTracks = show;

	showGraph(show);
	setInfo();

	redraw();
}
