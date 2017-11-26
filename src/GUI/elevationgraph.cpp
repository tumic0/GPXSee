#include <cmath>
#include "data/data.h"
#include "config.h"
#include "tooltip.h"
#include "elevationgraphitem.h"
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

	setYUnits(Metric);
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
	if (graph.size() < 2) {
		skipColor();
		return;
	}

	ElevationGraphItem *gi = new ElevationGraphItem(graph, _graphType);
	GraphView::addGraph(gi, path, type);

	if (type == Track) {
		_trackAscent += gi->ascent();
		_trackDescent += gi->descent();
		_trackMax = nMax(_trackMax, gi->max());
		_trackMin = nMin(_trackMin, gi->min());
	} else {
		_routeAscent += gi->ascent();
		_routeDescent += gi->descent();
		_routeMax = nMax(_routeMax, gi->max());
		_routeMin = nMin(_routeMin, gi->min());
	}
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

void ElevationGraph::setYUnits(Units units)
{
	if (units == Metric) {
		GraphView::setYUnits(tr("m"));
		setYScale(1);
	} else {
		GraphView::setYUnits(tr("ft"));
		setYScale(M2FT);
	}
}

void ElevationGraph::setUnits(Units units)
{
	setYUnits(units);
	setInfo();

	GraphView::setUnits(units);
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
