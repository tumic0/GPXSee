#include "gpx.h"
#include "heartrategraph.h"


HeartRateGraph::HeartRateGraph(QWidget *parent) : GraphTab(parent)
{
	_units = Metric;

	GraphView::setYUnits(tr("1/min"));
	setXLabel(tr("Distance"));
	setYLabel(tr("Heart rate"));

	setSliderPrecision(0);
}

void HeartRateGraph::setInfo()
{
	GraphView::addInfo(tr("Average"), QString::number(avg() * yScale(), 'f', 0)
	  + UNIT_SPACE + yUnits());
	GraphView::addInfo(tr("Maximum"), QString::number(max() * yScale(),  'f', 0)
	  + UNIT_SPACE + yUnits());
}

void HeartRateGraph::loadGPX(const GPX &gpx)
{
	for (int i = 0; i < gpx.tracks().count(); i++) {
		QVector<QPointF> data = gpx.tracks().at(i)->heartRate();
		qreal sum = 0, w = 0;

		if (data.count() < 2) {
			skipColor();
			continue;
		}

		for (int j = 1; j < data.size(); j++) {
			sum += data.at(j).y() * (data.at(j).x() - data.at(j-1).x());
			w += data.at(j).x() - data.at(j-1).x();
		}
		_avg.append(QPointF(gpx.tracks().at(i)->distance(), sum/w));

		loadData(data);
	}

	for (int i = 0; i < gpx.routes().count(); i++)
		skipColor();

	setXUnits();
	setInfo();

	redraw();
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
	_avg.clear();

	GraphView::clear();
}

void HeartRateGraph::setXUnits()
{
	if (_units == Metric) {
		if (bounds().width() < KMINM) {
			GraphView::setXUnits(tr("m"));
			setXScale(1);
		} else {
			GraphView::setXUnits(tr("km"));
			setXScale(M2KM);
		}
	} else {
		if (bounds().width() < MIINM) {
			GraphView::setXUnits(tr("ft"));
			setXScale(M2FT);
		} else {
			GraphView::setXUnits(tr("mi"));
			setXScale(M2MI);
		}
	}
}

void HeartRateGraph::setUnits(enum Units units)
{
	_units = units;

	setXUnits();
	setInfo();

	redraw();
}
