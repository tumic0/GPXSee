#include <QLocale>
#include "data/data.h"
#include "heartrategraphitem.h"
#include "heartrategraph.h"


HeartRateGraph::HeartRateGraph(QWidget *parent) : GraphTab(parent)
{
	_showTracks = false;

	GraphView::setYUnits(tr("bpm"));
	setYLabel(tr("Heart rate"));

	setSliderPrecision(0);
}

HeartRateGraph::~HeartRateGraph()
{
	qDeleteAll(_tracks);
}

void HeartRateGraph::setInfo()
{
	if (_showTracks) {
		QLocale l(QLocale::system());

#ifdef Q_OS_ANDROID
		GraphView::addInfo(tr("Avg"), l.toString(avg() * yScale(), 'f', 0)
		  + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Max"), l.toString(max() * yScale(), 'f', 0)
		  + UNIT_SPACE + yUnits());
#else // Q_OS_ANDROID
		GraphView::addInfo(tr("Average"), l.toString(avg() * yScale(), 'f',
		  0) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Maximum"), l.toString(max() * yScale(), 'f',
		  0) + UNIT_SPACE + yUnits());
#endif // Q_OS_ANDROID
	} else
		clearInfo();
}

QList<GraphItem*> HeartRateGraph::loadData(const Data &data)
{
	QList<GraphItem*> graphs;

	for (int i = 0; i < data.tracks().count(); i++) {
		const Track &track = data.tracks().at(i);
		const Graph &graph = track.heartRate();

		if (graph.isEmpty()) {
			_palette.nextColor();
			graphs.append(0);
		} else {
			HeartRateGraphItem *gi = new HeartRateGraphItem(graph, _graphType,
			  _width, _palette.nextColor());

			_tracks.append(gi);
			if (_showTracks)
				addGraph(gi);

			_avg.append(QPointF(track.distance(), gi->avg()));
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

qreal HeartRateGraph::avg() const
{
	qreal sum = 0, w = 0;

	for (int i = 0; i < _avg.size(); i++) {
		const QPointF &p = _avg.at(i);
		sum += p.y() * p.x();
		w += p.x();
	}

	return (sum / w);
}

void HeartRateGraph::clear()
{
	qDeleteAll(_tracks);
	_tracks.clear();

	_avg.clear();

	GraphTab::clear();
}

void HeartRateGraph::showTracks(bool show)
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
