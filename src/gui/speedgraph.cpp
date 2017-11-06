#include "config.h"
#include "data.h"
#include "tooltip.h"
#include "speedgraphitem.h"
#include "speedgraph.h"


SpeedGraph::SpeedGraph(QWidget *parent) : GraphTab(parent)
{
	_timeType = Total;
	_showTracks = true;

	setYUnits(Metric);
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
		const Track *track = data.tracks().at(i);
		const Graph &graph = track->speed();

		if (graph.size() < 2) {
			skipColor();
			continue;
		}

		SpeedGraphItem *gi = new SpeedGraphItem(graph, _graphType,
		  track->movingTime());
		gi->setTimeType(_timeType);
		GraphView::addGraph(gi, paths.at(i));

		_avg.append(QPointF(track->distance(), gi->avg()));
		_mavg.append(QPointF(track->distance(), gi->mavg()));
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
	const QList<QPointF> &list = (_timeType == Moving) ? _mavg : _avg;

	for (it = list.begin(); it != list.end(); it++) {
		sum += it->y() * it->x();
		w += it->x();
	}

	return (sum / w);
}

void SpeedGraph::clear()
{
	_avg.clear();
	_mavg.clear();

	GraphView::clear();
}

void SpeedGraph::setYUnits(Units units)
{
	if (units == Metric) {
		GraphView::setYUnits(tr("km/h"));
		setYScale(MS2KMH);
	} else {
		GraphView::setYUnits(tr("mi/h"));
		setYScale(MS2MIH);
	}
}

void SpeedGraph::setUnits(Units units)
{
	setYUnits(units);
	setInfo();

	GraphView::setUnits(units);
}

void SpeedGraph::setTimeType(enum TimeType type)
{
	_timeType = type;

	for (int i = 0; i < _graphs.size(); i++)
		static_cast<SpeedGraphItem*>(_graphs.at(i))->setTimeType(type);

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
