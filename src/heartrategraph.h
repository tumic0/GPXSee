#ifndef HEARTRATEGRAPH_H
#define HEARTRATEGRAPH_H

#include "graphview.h"
#include "units.h"

class GPX;

class HeartRateGraph : public GraphView
{
	Q_OBJECT

public:
	HeartRateGraph(QWidget *parent = 0);

	void loadGPX(const GPX &gpx);
	void clear();
	void setUnits(enum Units units);

	qreal avg() const;
	qreal max() const {return _max;}

private:
	void addInfo();

	qreal _max;
	QList<QPointF> _avg;
};

#endif // HEARTRATEGRAPH_H
