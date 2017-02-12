#include "config.h"
#include "data.h"
#include "speedgraph.h"


SpeedGraph::SpeedGraph(QWidget *parent) : GraphTab(parent)
{
	_units = Metric;
	_showTracks = true;

	setYUnits();
	setYLabel(tr("Speed"));

	setSliderPrecision(1);
}

void SpeedGraph::setInfo()
{
	if (_showTracks) {
		GraphView::addInfo(tr("Average"), QString::number(avg() * yScale(), 'f',
		  1) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Maximum"), QString::number(max() * yScale(), 'f',
		  1) + UNIT_SPACE + yUnits());
	} else
		clearInfo();
}

void SpeedGraph::loadData(const Data &data, const QList<PathItem *> &paths)
{
	for (int i = 0; i < data.tracks().count(); i++) {
		const Graph &graph = data.tracks().at(i)->speed();
		if (graph.size() < 2) {
			skipColor();
			continue;
		}

		_avg.append(QPointF(data.tracks().at(i)->distance(),
		  data.tracks().at(i)->distance() / data.tracks().at(i)->time()));
		_avgM.append(QPointF(data.tracks().at(i)->distance(),
		  data.tracks().at(i)->distance() / data.tracks().at(i)->movingTime()));

		GraphView::loadGraph(graph, paths.at(i));
	}

	for (int i = 0; i < data.routes().count(); i++)
		skipColor();

	setInfo();

	redraw();
}

qreal SpeedGraph::avg() const
{
	qreal sum = 0, w = 0;
	QList<QPointF>::const_iterator it;
	const QList<QPointF> &list = (_timeType == Moving) ? _avgM : _avg;

	for (it = list.begin(); it != list.end(); it++) {
		sum += it->y() * it->x();
		w += it->x();
	}

	return (sum / w);
}

void SpeedGraph::clear()
{
	_avg.clear();
	_avgM.clear();

	GraphView::clear();
}

void SpeedGraph::setYUnits()
{
	if (_units == Metric) {
		GraphView::setYUnits(tr("km/h"));
		setYScale(MS2KMH);
	} else {
		GraphView::setYUnits(tr("mi/h"));
		setYScale(MS2MIH);
	}
}

void SpeedGraph::setUnits(enum Units units)
{
	_units = units;

	setYUnits();
	setInfo();
	GraphView::setUnits(units);

	redraw();
}

void SpeedGraph::setTimeType(enum TimeType type)
{
	_timeType = type;

	setInfo();
	redraw();
}

void SpeedGraph::showTracks(bool show)
{
	_showTracks = show;

	showGraph(show);
	setInfo();

	redraw();
}
