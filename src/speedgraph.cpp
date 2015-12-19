#include "config.h"
#include "speedgraph.h"


SpeedGraph::SpeedGraph(QWidget *parent) : Graph(parent)
{
	_max = 0;

	Graph::setXLabel(tr("Distance"));
	Graph::setYLabel(tr("Speed"));
	Graph::setXUnits(tr("km"));
	Graph::setYUnits(tr("km/h"));
	Graph::setXScale(M2KM);
	Graph::setYScale(MS2KMH);
	Graph::setPrecision(1);
}

void SpeedGraph::addInfo()
{
	Graph::addInfo(tr("Average"), QString::number(avg() * _yScale, 'f', 1)
	  + THIN_SPACE + _yUnits);
	Graph::addInfo(tr("Maximum"), QString::number(_max * _yScale,  'f', 1)
	  + THIN_SPACE + _yUnits);
}

void SpeedGraph::loadGPX(const GPX &gpx)
{
	QVector<QPointF> data;
	qreal max = 0;


	gpx.speedGraph(data);
	if (data.isEmpty())
		return;

	_avg.append(QPointF(gpx.distance(), gpx.distance() / gpx.time()));

	for (int i = 0; i < data.size(); i++)
		max = qMax(max, data.at(i).y());
	_max = qMax(_max, max);

	addInfo();
	loadData(data);
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

	Graph::clear();
}

void SpeedGraph::setUnits(enum Units units)
{
	if (units == Metric) {
		Graph::setXUnits(tr("km"));
		Graph::setYUnits(tr("km/h"));
		Graph::setXScale(M2KM);
		Graph::setYScale(MS2KMH);
	} else {
		Graph::setXUnits(tr("mi"));
		Graph::setYUnits(tr("mi/h"));
		Graph::setXScale(M2MI);
		Graph::setYScale(MS2MIH);
	}

	clearInfo();
	addInfo();

	redraw();
}
