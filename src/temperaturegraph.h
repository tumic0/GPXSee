#ifndef TEMPERATUREGRAPH_H
#define TEMPERATUREGRAPH_H

#include "graphtab.h"

class GPX;

class TemperatureGraph : public GraphTab
{
	Q_OBJECT

public:
	TemperatureGraph(QWidget *parent = 0);

	QString label() const {return tr("Temperature");}
	void loadGPX(const GPX &gpx);
	void clear();
	void setUnits(enum Units units);

private:
	qreal avg() const;
	qreal min() const {return bounds().top();}
	qreal max() const {return bounds().bottom();}
	void setXUnits();
	void setYUnits();
	void addInfo();

	QList<QPointF> _avg;
	enum Units _units;
};

#endif // TEMPERATUREGRAPH_H
