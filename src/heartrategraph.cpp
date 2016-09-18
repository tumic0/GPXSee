#include "gpx.h"
#include "heartrategraph.h"


HeartRateGraph::HeartRateGraph(QWidget *parent) : GraphTab(parent)
{
	_units = Metric;
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

void HeartRateGraph::loadGPX(const GPX &gpx, const QList<PathItem *> &paths)
{
	for (int i = 0; i < gpx.tracks().count(); i++) {
		const Graph &graph = gpx.tracks().at(i)->heartRate();
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
