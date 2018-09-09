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
		const Graph &graph = data.tracks().at(i)->ratio();

		if (graph.size() < 2) {
			skipColor();
			graphs.append(0);
		} else {
			GearRatioGraphItem *gi = new GearRatioGraphItem(graph, _graphType);
			GraphView::addGraph(gi);

			for (QMap<qreal, qreal>::const_iterator it = gi->map().constBegin();
			  it != gi->map().constEnd(); ++it)
				_map.insert(it.key(), _map.value(it.key()) + it.value());
			graphs.append(gi);
		}
	}

	for (int i = 0; i < data.routes().count(); i++) {
		skipColor();
		graphs.append(0);
	}

	setInfo();
	redraw();

	return graphs;
}

qreal GearRatioGraph::top() const
{
	qreal key = NAN, val = NAN;

	for (QMap<qreal, qreal>::const_iterator it = _map.constBegin();
	  it != _map.constEnd(); ++it) {
		if (it == _map.constBegin()) {
			val = it.value();
			key = it.key();
		} else if (it.value() > val) {
			val = it.value();
			key = it.key();
		}
	}

	return key;
}

void GearRatioGraph::clear()
{
	_map.clear();

	GraphView::clear();
}

void GearRatioGraph::showTracks(bool show)
{
	_showTracks = show;

	showGraph(show);
	setInfo();

	redraw();
}
