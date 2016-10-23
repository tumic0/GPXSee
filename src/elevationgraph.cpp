#include <cmath>
#include "config.h"
#include "data.h"
#include "elevationgraph.h"


static qreal nMin(qreal a, qreal b)
{
	if (!std::isnan(a) && !std::isnan(b))
		return qMin(a, b);
	else if (!std::isnan(a))
		return a;
	else if (!std::isnan(b))
		return b;
	else
		return NAN;
}

static qreal nMax(qreal a, qreal b)
{
	if (!std::isnan(a) && !std::isnan(b))
		return qMax(a, b);
	else if (!std::isnan(a))
		return a;
	else if (!std::isnan(b))
		return b;
	else
		return NAN;
}

ElevationGraph::ElevationGraph(QWidget *parent) : GraphTab(parent)
{
	_trackAscent = 0;
	_routeAscent = 0;
	_trackDescent = 0;
	_routeDescent = 0;
	_trackMin = NAN;
	_trackMax = NAN;
	_routeMin = NAN;
	_routeMax = NAN;

	_showRoutes = true;
	_showTracks = true;

	_units = Metric;

	setYUnits();
	setYLabel(tr("Elevation"));

	setMinYRange(50.0);
}

void ElevationGraph::setInfo()
{
	if (std::isnan(max()) || std::isnan(min()))
		clearInfo();
	else {
		GraphView::addInfo(tr("Ascent"), QString::number(ascent() * yScale(),
		  'f', 0) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Descent"), QString::number(descent() * yScale(),
		  'f', 0) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Maximum"), QString::number(max() * yScale(), 'f',
		  0) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Minimum"), QString::number(min() * yScale(), 'f',
		  0) + UNIT_SPACE + yUnits());
	}
}

void ElevationGraph::loadGraph(const Graph &graph, Type type, PathItem *path)
{
	qreal ascent = 0, descent = 0;
	qreal min, max;

	if (graph.size() < 2) {
		skipColor();
		return;
	}

	max = min = graph.at(0).y();
	for (int j = 1; j < graph.size(); j++) {
		qreal cur = graph.at(j).y();
		qreal prev = graph.at(j-1).y();

		if (cur > prev)
			ascent += cur - prev;
		if (cur < prev)
			descent += prev - cur;

		if (cur < min)
			min = cur;
		if (cur > max)
			max = cur;
	}

	if (type == Track) {
		_trackAscent += ascent;
		_trackDescent += descent;
		_trackMax = nMax(_trackMax, max);
		_trackMin = nMin(_trackMin, min);
	} else {
		_routeAscent += ascent;
		_routeDescent += descent;
		_routeMax = nMax(_routeMax, max);
		_routeMin = nMin(_routeMin, min);
	}

	GraphView::loadGraph(graph, path, type);
}

void ElevationGraph::loadData(const Data &data, const QList<PathItem *> &paths)
{
	int p = 0;

	for (int i = 0; i < data.tracks().count(); i++)
		loadGraph(data.tracks().at(i)->elevation(), Track, paths.at(p++));
	for (int i = 0; i < data.routes().count(); i++)
		loadGraph(data.routes().at(i)->elevation(), Route, paths.at(p++));

	setInfo();

	redraw();
}

void ElevationGraph::clear()
{
	_trackAscent = 0;
	_routeAscent = 0;
	_trackDescent = 0;
	_routeDescent = 0;
	_trackMin = NAN;
	_trackMax = NAN;
	_routeMin = NAN;
	_routeMax = NAN;

	GraphView::clear();
}

void ElevationGraph::setYUnits()
{
	if (_units == Metric) {
		GraphView::setYUnits(tr("m"));
		setYScale(1);
	} else {
		GraphView::setYUnits(tr("ft"));
		setYScale(M2FT);
	}
}

void ElevationGraph::setUnits(enum Units units)
{
	_units = units;

	setYUnits();
	setInfo();
	GraphView::setUnits(units);

	redraw();
}

void ElevationGraph::showTracks(bool show)
{
	_showTracks = show;

	setInfo();
	showGraph(show, Track);

	redraw();
}

void ElevationGraph::showRoutes(bool show)
{
	_showRoutes = show;

	showGraph(show, Route);
	setInfo();

	redraw();
}

qreal ElevationGraph::ascent() const
{
	qreal val = 0;

	if (_showRoutes)
		val += _routeAscent;
	if (_showTracks)
		val += _trackAscent;

	return val;
}

qreal ElevationGraph::descent() const
{
	qreal val = 0;

	if (_showRoutes)
		val += _routeDescent;
	if (_showTracks)
		val += _trackDescent;

	return val;
}

qreal ElevationGraph::max() const
{
	qreal val;

	if (_showRoutes && _showTracks)
		val = nMax(_routeMax, _trackMax);
	else if (_showTracks)
		val = _trackMax;
	else if (_showRoutes)
		val = _routeMax;
	else
		val = NAN;

	return val;
}

qreal ElevationGraph::min() const
{
	qreal val;

	if (_showRoutes && _showTracks)
		val = nMin(_routeMin, _trackMin);
	else if (_showTracks)
		val = _trackMin;
	else if (_showRoutes)
		val = _routeMin;
	else
		val = NAN;

	return val;
}
