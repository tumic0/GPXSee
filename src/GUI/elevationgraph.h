#ifndef ELEVATIONGRAPH_H
#define ELEVATIONGRAPH_H

#include "graphtab.h"

class ElevationGraph : public GraphTab
{
	Q_OBJECT

public:
	ElevationGraph(QWidget *parent = 0);

	QString label() const {return tr("Elevation");}
	QList<GraphItem*> loadData(const Data &data);
	void clear();
	void setUnits(enum Units units);
	void showTracks(bool show);
	void showRoutes(bool show);

private:
	enum Type {Track, Route};

	qreal max() const;
	qreal min() const;
	qreal ascent() const;
	qreal descent() const;

	void setYUnits(Units units);
	void setInfo();

	GraphItem *loadGraph(const Graph &graph, Type type);

	qreal _trackAscent, _trackDescent;
	qreal _routeAscent, _routeDescent;
	qreal _trackMax, _routeMax;
	qreal _trackMin, _routeMin;

	bool _showTracks, _showRoutes;
};

#endif // ELEVATIONGRAPH_H
