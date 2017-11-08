#include "data.h"
#include "cadencegraphitem.h"
#include "cadencegraph.h"


CadenceGraph::CadenceGraph(GeoItems &geoItems, QWidget *parent)
	: GraphTab(geoItems, parent)
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

void CadenceGraph::addTrack(const Track &track, TrackItem *item)
{
	const Graph &graph = track.cadence();

	if (graph.size() >= 2) {
		CadenceGraphItem *gi = new CadenceGraphItem(graph, _graphType);
		GraphView::addGraph(gi, (PathItem*)item);

		_avg.append(QPointF(track.distance(), gi->avg()));

		redraw();
	}
}
