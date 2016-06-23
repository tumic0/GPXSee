#ifndef ELEVATIONGRAPH_H
#define ELEVATIONGRAPH_H

#include "graphtab.h"

class GPX;

class ElevationGraph : public GraphTab
{
	Q_OBJECT

public:
	ElevationGraph(QWidget *parent = 0);

	QString label() const {return tr("Elevation");}
	void loadGPX(const GPX &gpx);
	void clear();
	void setUnits(enum Units units);

private:
	qreal ascent() const {return _ascent;}
	qreal descent() const {return _descent;}
	qreal max() const {return bounds().bottom();}
	qreal min() const {return bounds().top();}

	void setXUnits();
	void setYUnits();
	void addInfo();

	qreal _ascent, _descent;
	enum Units _units;
};

#endif // ELEVATIONGRAPH_H
