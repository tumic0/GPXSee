#ifndef TEMPERATUREGRAPH_H
#define TEMPERATUREGRAPH_H

#include "graphtab.h"

class TemperatureGraphItem;

class TemperatureGraph : public GraphTab
{
	Q_OBJECT

public:
	TemperatureGraph(QWidget *parent = 0);
	~TemperatureGraph();

	QString label() const {return tr("Temperature");}
	QList<GraphItem*> loadData(const Data &data);
	void clear();
	void setUnits(enum Units units);
	void showTracks(bool show);

private:
	qreal avg() const;
	qreal min() const {return bounds().top();}
	qreal max() const {return bounds().bottom();}
	void setYUnits(Units units);
	void setInfo();

	QVector<QPointF> _avg;

	bool _showTracks;
	QList<TemperatureGraphItem *> _tracks;
};

#endif // TEMPERATUREGRAPH_H
