#include <cmath>
#include <QLocale>
#include "data/data.h"
#include "tooltip.h"
#include "elevationgraphitem.h"
#include "elevationgraph.h"


static qreal nMin(qreal a, qreal b)
{
	if (std::isnan(a))
		return std::isnan(b) ? NAN : b;
	else
		return std::isnan(b) ? a : qMin(a, b);
}

static qreal nMax(qreal a, qreal b)
{
	if (std::isnan(a))
		return std::isnan(b) ? NAN : b;
	else
		return std::isnan(b) ? a : qMax(a, b);
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

ElevationGraph::~ElevationGraph()
{
	qDeleteAll(_tracks);
	qDeleteAll(_routes);
}

void ElevationGraph::setInfo()
{
	if (std::isnan(max()) || std::isnan(min()))
		clearInfo();
	else {
		QLocale l(QLocale::system());

		GraphView::addInfo(tr("Ascent"), l.toString(ascent() * yScale(),
		  'f', 0) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Descent"), l.toString(descent() * yScale(),
		  'f', 0) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Maximum"), l.toString(max() * yScale(), 'f',
		  0) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Minimum"), l.toString(min() * yScale(), 'f',
		  0) + UNIT_SPACE + yUnits());
	}
}

GraphItem *ElevationGraph::loadGraph(const Graph &graph, Type type)
{
	if (!graph.isValid()) {
		_palette.nextColor();
		return 0;
	}

	ElevationGraphItem *gi = new ElevationGraphItem(graph, _graphType, _width,
	  _palette.nextColor());
	gi->setUnits(_units);

	if (type == Track) {
		_tracks.append(gi);
		if (_showTracks)
			addGraph(gi);

		_trackAscent += gi->ascent();
		_trackDescent += gi->descent();
		_trackMax = nMax(_trackMax, gi->max());
		_trackMin = nMin(_trackMin, gi->min());
	} else {
		_routes.append(gi);
		if (_showRoutes)
			addGraph(gi);

		_routeAscent += gi->ascent();
		_routeDescent += gi->descent();
		_routeMax = nMax(_routeMax, gi->max());
		_routeMin = nMin(_routeMin, gi->min());
	}

	return gi;
}

QList<GraphItem*> ElevationGraph::loadData(const Data &data)
{
	QList<GraphItem*> graphs;

	for (int i = 0; i < data.tracks().count(); i++)
		graphs.append(loadGraph(data.tracks().at(i).elevation(), Track));
	for (int i = 0; i < data.routes().count(); i++)
		graphs.append(loadGraph(data.routes().at(i).elevation(), Route));
	for (int i = 0; i < data.areas().count(); i++)
		_palette.nextColor();

	setInfo();
	redraw();

	return graphs;
}

void ElevationGraph::clear()
{
	qDeleteAll(_tracks);
	_tracks.clear();
	qDeleteAll(_routes);
	_routes.clear();

	_trackAscent = 0;
	_routeAscent = 0;
	_trackDescent = 0;
	_routeDescent = 0;
	_trackMin = NAN;
	_trackMax = NAN;
	_routeMin = NAN;
	_routeMax = NAN;

	GraphTab::clear();
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

void ElevationGraph::showItems(const QList<ElevationGraphItem *> &list,
  bool show)
{
	for (int i = 0; i < list.size(); i++) {
		if (show)
			addGraph(list.at(i));
		else
			removeGraph(list.at(i));
	}
}

void ElevationGraph::showTracks(bool show)
{
	_showTracks = show;

	showItems(_tracks, show);
	setInfo();

	redraw();
}

void ElevationGraph::showRoutes(bool show)
{
	_showRoutes = show;

	showItems(_routes, show);
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
