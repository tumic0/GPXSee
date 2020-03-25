#include <QLocale>
#include "data/data.h"
#include "tooltip.h"
#include "format.h"
#include "speedgraphitem.h"
#include "speedgraph.h"


SpeedGraph::SpeedGraph(QWidget *parent) : GraphTab(parent)
{
	_units = Metric;
	_timeType = Total;
	_showTracks = true;

	setYUnits();
	setYLabel(tr("Speed"));

	setSliderPrecision(1);
}

SpeedGraph::~SpeedGraph()
{
	qDeleteAll(_tracks);
}

void SpeedGraph::setInfo()
{
	if (_showTracks) {
		QLocale l(QLocale::system());
		QString pace = Format::timeSpan((3600.0 / (avg() * yScale())), false);
		QString pu = (_units == Metric) ? tr("min/km") : (_units == Imperial) ?
		  tr("min/mi") : tr("min/nmi");

		GraphView::addInfo(tr("Average"), l.toString(avg() * yScale(), 'f',
		  1) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Maximum"), l.toString(max() * yScale(), 'f',
		  1) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Pace"), pace + UNIT_SPACE + pu);
	} else
		clearInfo();
}

GraphItem *SpeedGraph::loadGraph(const Graph &graph, const Track &track,
  const QColor &color, bool primary)
{
	if (!graph.isValid())
		return 0;

	SpeedGraphItem *gi = new SpeedGraphItem(graph, _graphType, _width,
	  color, primary ? Qt::SolidLine : Qt::DashLine, track.movingTime());
	gi->setTimeType(_timeType);
	gi->setUnits(_units);

	_tracks.append(gi);
	if (_showTracks)
		addGraph(gi);

	if (primary) {
		_avg.append(QPointF(track.distance(), gi->avg()));
		_mavg.append(QPointF(track.distance(), gi->mavg()));
	}

	return gi;
}

QList<GraphItem*> SpeedGraph::loadData(const Data &data)
{
	QList<GraphItem*> graphs;

	for (int i = 0; i < data.tracks().count(); i++) {
		GraphItem *primary, *secondary;
		QColor color(_palette.nextColor());
		const Track &track = data.tracks().at(i);
		const GraphPair &gp = track.speed();

		primary = loadGraph(gp.primary(), track, color, true);
		secondary = primary
		  ? loadGraph(gp.secondary(), track, color, false) : 0;
		if (primary && secondary)
			primary->setSecondaryGraph(secondary);

		graphs.append(primary);
	}

	for (int i = 0; i < data.routes().count(); i++) {
		_palette.nextColor();
		graphs.append(0);
	}

	for (int i = 0; i < data.areas().count(); i++)
		_palette.nextColor();

	setInfo();
	redraw();

	return graphs;
}

qreal SpeedGraph::avg() const
{
	qreal sum = 0, w = 0;
	const QVector<QPointF> &vector = (_timeType == Moving) ? _mavg : _avg;

	for (int i = 0; i < vector.size(); i++) {
		const QPointF &p = vector.at(i);
		sum += p.y() * p.x();
		w += p.x();
	}

	return (sum / w);
}

void SpeedGraph::clear()
{
	qDeleteAll(_tracks);
	_tracks.clear();

	_avg.clear();
	_mavg.clear();

	GraphTab::clear();
}

void SpeedGraph::setYUnits()
{
	if (_units == Nautical) {
		GraphView::setYUnits(tr("kn"));
		setYScale(MS2KN);
	} else if (_units == Imperial) {
		GraphView::setYUnits(tr("mi/h"));
		setYScale(MS2MIH);
	} else {
		GraphView::setYUnits(tr("km/h"));
		setYScale(MS2KMH);
	}
}

void SpeedGraph::setUnits(Units units)
{
	_units = units;

	setYUnits();
	setInfo();

	GraphView::setUnits(units);
}

void SpeedGraph::setTimeType(enum TimeType type)
{
	_timeType = type;

	for (int i = 0; i < _tracks.size(); i++)
		_tracks.at(i)->setTimeType(type);

	setInfo();
	redraw();
}

void SpeedGraph::showTracks(bool show)
{
	_showTracks = show;

	for (int i = 0; i < _tracks.size(); i++) {
		if (show)
			addGraph(_tracks.at(i));
		else
			removeGraph(_tracks.at(i));
	}

	setInfo();

	redraw();
}
