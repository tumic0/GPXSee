#ifndef ELEVATIONGRAPH_H
#define ELEVATIONGRAPH_H

#include "graphtab.h"

class ElevationGraph : public GraphTab
{
	Q_OBJECT

public:
	ElevationGraph(GeoItems &geoItems, QWidget *parent = 0);

	QString label() const {return tr("Elevation");}
	void loadData(const Data &data, const QList<PathItem *> &paths);
	void clear();
	void setUnits(enum Units units);
	void showTracks(bool show);
	void showRoutes(bool show);

public slots:
	virtual void addTrack(const Track &track, TrackItem *item);
	virtual void addRoute(const Route &route, RouteItem *item);

private:
	enum Type {TRACK_TYPE, ROUTE_TYPE};

	qreal max() const;
	qreal min() const;
	qreal ascent() const;
	qreal descent() const;

	void setYUnits(Units units);
	void setInfo();

	void loadGraph(const Graph &graph, Type type, PathItem *path);

	qreal _trackAscent, _trackDescent;
	qreal _routeAscent, _routeDescent;
	qreal _trackMax, _routeMax;
	qreal _trackMin, _routeMin;

	bool _showTracks, _showRoutes;
};

#endif // ELEVATIONGRAPH_H
