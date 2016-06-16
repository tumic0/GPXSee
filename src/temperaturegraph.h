#ifndef TEMPERATUREGRAPH_H
#define TEMPERATUREGRAPH_H

#include "units.h"
#include "graphview.h"

class GPX;

class TemperatureGraph : public GraphView
{
	Q_OBJECT

public:
	TemperatureGraph(QWidget *parent = 0);

	void loadGPX(const GPX &gpx);
	void clear();
	void setUnits(enum Units units);

	qreal avg() const;
	qreal min() const {return bounds().top();}
	qreal max() const {return bounds().bottom();}

private:
	void setXUnits();
	void setYUnits();
	void addInfo();

	QList<QPointF> _avg;
	enum Units _units;
};

#endif // TEMPERATUREGRAPH_H
