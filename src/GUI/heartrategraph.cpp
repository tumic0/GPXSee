#include "data/data.h"
#include "heartrategraphitem.h"
#include "heartrategraph.h"


HeartRateGraph::HeartRateGraph(QWidget *parent) : GraphTab(parent)
{
	_showTracks = true;

	GraphView::setYUnits(tr("1/min"));
	setYLabel(tr("Heart rate"));

	setSliderPrecision(0);
}

void HeartRateGraph::setInfo()
{
	if (_showTracks) {
		GraphView::addInfo(tr("Average"), QString::number(avg() * yScale(), 'f',
		  0) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Maximum"), QString::number(max() * yScale(), 'f',
		  0) + UNIT_SPACE + yUnits());
	} else
		clearInfo();
}

void HeartRateGraph::loadData(const Data &data, const QList<PathItem *> &paths)
{
	for (int i = 0; i < data.tracks().count(); i++) {
		const Graph &graph = data.tracks().at(i)->heartRate();

		if (graph.size() < 2) {
			skipColor();
			continue;
		}

		HeartRateGraphItem *gi = new HeartRateGraphItem(graph, _graphType);
		GraphView::addGraph(gi, paths.at(i));

		_avg.append(QPointF(data.tracks().at(i)->distance(), gi->avg()));
	}

	for (int i = 0; i < data.routes().count(); i++)
		skipColor();

	setInfo();

	redraw();
}

qreal HeartRateGraph::avg() const
{
	qreal sum = 0, w = 0;
	QList<QPointF>::const_iterator it;

	for (it = _avg.begin(); it != _avg.end(); it++) {
		sum += it->y() * it->x();
		w += it->x();
	}

	return (sum / w);
}

void HeartRateGraph::clear()
{
	_avg.clear();

	GraphView::clear();
}

void HeartRateGraph::showTracks(bool show)
{
	_showTracks = show;

	showGraph(show);
	setInfo();

	redraw();
}
