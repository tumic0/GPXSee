#include "data.h"
#include "powergraphitem.h"
#include "powergraph.h"


PowerGraph::PowerGraph(GeoItems &geoItems, QWidget *parent)
	: GraphTab(geoItems, parent)
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

void PowerGraph::addTrack(const Track &track, TrackItem *item)
{
	const Graph &graph = track.power();

	if (graph.size() >= 2) {
		PowerGraphItem *gi = new PowerGraphItem(graph, _graphType);
		GraphView::addGraph(gi, item);

		_avg.append(QPointF(track.distance(), gi->avg()));

		setInfo();
		redraw();
	}
}
