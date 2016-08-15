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
	void showTracks(bool show);
	void showRoutes(bool show);

private:
	enum Type {Track, Route};

	qreal max() const;
	qreal min() const;
	qreal ascent() const;
	qreal descent() const;

	void setXUnits();
	void setYUnits();
	void setInfo();

	void loadPath(const QVector<QPointF> &data, Type type);

	qreal _trackAscent, _trackDescent;
	qreal _routeAscent, _routeDescent;
	qreal _trackMax, _routeMax;
	qreal _trackMin, _routeMin;

	enum Units _units;
	bool _showTracks, _showRoutes;
};

#endif // ELEVATIONGRAPH_H
