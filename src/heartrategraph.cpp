#include "gpx.h"
#include "heartrategraph.h"


HeartRateGraph::HeartRateGraph(QWidget *parent) : GraphView(parent)
{
	_max = 0;

	setXLabel(tr("Distance"));
	setYLabel(tr("Heart rate"));
	setXUnits(tr("km"));
	setYUnits(tr("1/min"));
	setXScale(M2KM);
	setPrecision(0);
}

void HeartRateGraph::addInfo()
{
	GraphView::addInfo(tr("Average"), QString::number(avg() * _yScale, 'f', 0)
	  + UNIT_SPACE + _yUnits);
	GraphView::addInfo(tr("Maximum"), QString::number(_max * _yScale,  'f', 0)
	  + UNIT_SPACE + _yUnits);
}

void HeartRateGraph::loadGPX(const GPX &gpx)
{
	for (int i = 0; i < gpx.trackCount(); i++) {
		QVector<QPointF> data;
		qreal max = 0, sum = 0, w = 0;

		gpx.track(i).heartRateGraph(data);
		if (data.count() < 2) {
			skipColor();
			continue;
		}

		for (int j = 1; j < data.size(); j++) {
			sum += data.at(j).y() * (data.at(j).x() - data.at(j-1).x());
			w += data.at(j).x() - data.at(j-1).x();
		}
		_avg.append(QPointF(gpx.track(i).distance(), sum/w));

		for (int j = 0; j < data.size(); j++)
			max = qMax(max, data.at(j).y());
		_max = qMax(_max, max);

		addInfo();
		loadData(data);
	}
}

qreal HeartRateGraph::avg() const
{
	qreal sum = 0, w = 0;
	QList<QPointF>::const_iterator it;

	for (it = _avg.begin(); it != _avg.end(); it++) {
		sum += it->y() * it->x();
		w += it->x();
	}

	return (sum / w);
}

void HeartRateGraph::clear()
{
	_max = 0;
	_avg.clear();

	GraphView::clear();
}

void HeartRateGraph::setUnits(enum Units units)
{
	if (units == Metric) {
		setXUnits(tr("km"));
		setXScale(M2KM);
	} else {
		setXUnits(tr("mi"));
		setXScale(M2MI);
	}

	clearInfo();
	addInfo();

	redraw();
}
