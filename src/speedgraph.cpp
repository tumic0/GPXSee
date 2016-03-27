#include "config.h"
#include "gpx.h"
#include "speedgraph.h"


SpeedGraph::SpeedGraph(QWidget *parent) : GraphView(parent)
{
	setXLabel(tr("Distance"));
	setYLabel(tr("Speed"));
	setXUnits(tr("km"));
	setYUnits(tr("km/h"));
	setXScale(M2KM);
	setYScale(MS2KMH);
	setSliderPrecision(1);
}

void SpeedGraph::addInfo()
{
	GraphView::addInfo(tr("Average"), QString::number(avg() * yScale(), 'f', 1)
	  + UNIT_SPACE + yUnits());
	GraphView::addInfo(tr("Maximum"), QString::number(max() * yScale(),  'f', 1)
	  + UNIT_SPACE + yUnits());

	redraw();
}

void SpeedGraph::loadGPX(const GPX &gpx)
{
	for (int i = 0; i < gpx.trackCount(); i++) {
		QVector<QPointF> data;

		gpx.track(i).speedGraph(data);
		if (data.count() < 2) {
			skipColor();
			continue;
		}

		_avg.append(QPointF(gpx.track(i).distance(), gpx.track(i).distance()
		  / gpx.track(i).time()));

		loadData(data);
	}

	addInfo();
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
	_avg.clear();

	GraphView::clear();
}

void SpeedGraph::setUnits(enum Units units)
{
	if (units == Metric) {
		setXUnits(tr("km"));
		setYUnits(tr("km/h"));
		setXScale(M2KM);
		setYScale(MS2KMH);
	} else {
		setXUnits(tr("mi"));
		setYUnits(tr("mi/h"));
		setXScale(M2MI);
		setYScale(MS2MIH);
	}

	clearInfo();
	addInfo();
}
