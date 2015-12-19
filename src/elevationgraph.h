#ifndef ELEVATIONGRAPH_H
#define ELEVATIONGRAPH_H

#include "graph.h"
#include "gpx.h"
#include "units.h"

class ElevationGraph : public Graph
{
	Q_OBJECT

public:
	ElevationGraph(QWidget *parent = 0);

	void loadGPX(const GPX &gpx);
	void clear();
	void setUnits(enum Units units);

	qreal ascent() const {return _ascent;}
	qreal descent() const {return _descent;}
	qreal max() const {return _max;}
	qreal min() const {return _min;}

private:
	void addInfo();

	qreal _ascent, _descent;
	qreal _max, _min;
};

#endif // ELEVATIONGRAPH_H
