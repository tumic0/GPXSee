#include "gpx.h"
#include "temperaturegraph.h"


TemperatureGraph::TemperatureGraph(QWidget *parent) : GraphTab(parent)
{
	_units = Metric;
	_showTracks = true;

	setYUnits();
	setXLabel(tr("Distance"));
	setYLabel(tr("Temperature"));

	setSliderPrecision(1);
}

void TemperatureGraph::setInfo()
{
	if (_showTracks) {
		GraphView::addInfo(tr("Average"), QString::number(avg() * yScale()
		  + yOffset(), 'f', 1) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Minimum"), QString::number(min() * yScale()
		  + yOffset(),  'f', 1) + UNIT_SPACE + yUnits());
		GraphView::addInfo(tr("Maximum"), QString::number(max() * yScale()
		  + yOffset(),  'f', 1) + UNIT_SPACE + yUnits());
	} else
		clearInfo();
}

void TemperatureGraph::loadGPX(const GPX &gpx)
{
	for (int i = 0; i < gpx.tracks().count(); i++) {
		QVector<QPointF> data = gpx.tracks().at(i)->temperature();
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

qreal TemperatureGraph::avg() const
{
	qreal sum = 0, w = 0;
	QList<QPointF>::const_iterator it;

	for (it = _avg.begin(); it != _avg.end(); it++) {
		sum += it->y() * it->x();
		w += it->x();
	}

	return (sum / w);
}

void TemperatureGraph::clear()
{
	_avg.clear();

	GraphView::clear();
}

void TemperatureGraph::setXUnits()
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

void TemperatureGraph::setYUnits()
{
	if (_units == Metric) {
		GraphView::setYUnits(QChar(0x00B0) + tr("C"));
		setYScale(1);
		setYOffset(0);
	} else {
		GraphView::setYUnits(QChar(0x00B0) + tr("F"));
		setYScale(C2FS);
		setYOffset(C2FO);
	}
}

void TemperatureGraph::setUnits(enum Units units)
{
	_units = units;

	setXUnits();
	setYUnits();
	setInfo();

	redraw();
}

void TemperatureGraph::showTracks(bool show)
{
	_showTracks = show;

	showGraph(show);
	setXUnits();
	setInfo();

	redraw();
}
