#include <QLocale>
#include "data/data.h"
#include "gearratiographitem.h"
#include "gearratiograph.h"


GearRatioGraph::GearRatioGraph(QWidget *parent) : GraphTab(parent)
{
	_showTracks = true;

	GraphView::setYUnits("");
	setYLabel(tr("Gear ratio"));

	setSliderPrecision(2);
}

GearRatioGraph::~GearRatioGraph()
{
	qDeleteAll(_tracks);
}

void GearRatioGraph::setInfo()
{
	if (_showTracks) {
		QLocale l(QLocale::system());

		GraphView::addInfo(tr("Most used"), l.toString(top() * yScale(),
		  'f', 2) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Minimum"), l.toString(min() * yScale(), 'f',
		  2) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Maximum"), l.toString(max() * yScale(), 'f',
		  2) + UNIT_SPACE + yUnits());
	} else
		clearInfo();
}

QList<GraphItem*> GearRatioGraph::loadData(const Data &data)
{
	QList<GraphItem*> graphs;

	for (int i = 0; i < data.tracks().count(); i++) {
		const Graph &graph = data.tracks().at(i).ratio();

		if (!graph.isValid()) {
			_palette.nextColor();
			graphs.append(0);
		} else {
			GearRatioGraphItem *gi = new GearRatioGraphItem(graph, _graphType,
			  _width, _palette.nextColor());

			_tracks.append(gi);
			if (_showTracks)
				addGraph(gi);

			for (QMap<qreal, qreal>::const_iterator it = gi->map().constBegin();
			  it != gi->map().constEnd(); ++it)
				_map.insert(it.key(), _map.value(it.key()) + it.value());
			graphs.append(gi);
		}
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

qreal GearRatioGraph::top() const
{
	qreal key = NAN, val = NAN;

	for (QMap<qreal, qreal>::const_iterator it = _map.constBegin();
	  it != _map.constEnd(); ++it) {
		if (std::isnan(val) || it.value() > val) {
			val = it.value();
			key = it.key();
		}
	}

	return key;
}

void GearRatioGraph::clear()
{
	qDeleteAll(_tracks);
	_tracks.clear();

	_map.clear();

	GraphTab::clear();
}

void GearRatioGraph::showTracks(bool show)
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
