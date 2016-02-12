#include "config.h"
#include "gpx.h"
#include "speedgraph.h"


SpeedGraph::SpeedGraph(QWidget *parent) : GraphView(parent)
{
	_max = 0;

	GraphView::setXLabel(tr("Distance"));
	GraphView::setYLabel(tr("Speed"));
	GraphView::setXUnits(tr("km"));
	GraphView::setYUnits(tr("km/h"));
	GraphView::setXScale(M2KM);
	GraphView::setYScale(MS2KMH);
	GraphView::setPrecision(1);
}

void SpeedGraph::addInfo()
{
	GraphView::addInfo(tr("Average"), QString::number(avg() * _yScale, 'f', 1)
	  + THIN_SPACE + _yUnits);
	GraphView::addInfo(tr("Maximum"), QString::number(_max * _yScale,  'f', 1)
	  + THIN_SPACE + _yUnits);
}

void SpeedGraph::loadGPX(const GPX &gpx)
{
	for (int i = 0; i < gpx.trackCount(); i++) {
		QVector<QPointF> data;
		qreal max = 0;

		gpx.track(i).speedGraph(data);
		if (data.isEmpty())
			return;

		_avg.append(QPointF(gpx.track(i).distance(), gpx.track(i).distance()
		  / gpx.track(i).time()));

		for (int i = 0; i < data.size(); i++)
			max = qMax(max, data.at(i).y());
		_max = qMax(_max, max);

		addInfo();
		loadData(data);
	}
}

qreal SpeedGraph::avg() const
{
	qreal sum = 0, w = 0;
	QList<QPointF>::const_iterator it;

	for (it = _avg.begin(); it != _avg.end(); it++) {
		sum += it->y() * it->x();
		w += it->x();
	}

	return (sum / w);
}

void SpeedGraph::clear()
{
	_max = 0;
	_avg.clear();

	GraphView::clear();
}

void SpeedGraph::setUnits(enum Units units)
{
	if (units == Metric) {
		GraphView::setXUnits(tr("km"));
		GraphView::setYUnits(tr("km/h"));
		GraphView::setXScale(M2KM);
		GraphView::setYScale(MS2KMH);
	} else {
		GraphView::setXUnits(tr("mi"));
		GraphView::setYUnits(tr("mi/h"));
		GraphView::setXScale(M2MI);
		GraphView::setYScale(MS2MIH);
	}

	clearInfo();
	addInfo();

	redraw();
}
