#include "data.h"
#include "heartrategraphitem.h"
#include "heartrategraph.h"


HeartRateGraph::HeartRateGraph(GeoItems &geoItems, QWidget *parent)
	: GraphTab(geoItems, parent)
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

void HeartRateGraph::addTrack(const Track &track, TrackItem *item)
{
	const Graph &graph = track.heartRate();

	if (graph.size() >= 2) {
		HeartRateGraphItem *gi = new HeartRateGraphItem(graph, _graphType);
		GraphView::addGraph(gi, item);

		_avg.append(QPointF(track.distance(), gi->avg()));

		setInfo();
		redraw();
	}
}
