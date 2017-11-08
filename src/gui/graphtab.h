#ifndef GRAPHTAB_H
#define GRAPHTAB_H

#include <QList>
#include "graphview.h"
#include "units.h"
#include "timetype.h"
#include "geoitems/geoitems.h"

class Data;
class PathItem;

class Track;
class TrackItem;

class GraphTab : public GraphView
{
	Q_OBJECT

public:
	GraphTab(GeoItems &geoItems, QWidget *parent = 0)
		: GraphView(parent)
		, _geoItems(geoItems)
	{
		setFrameShape(QFrame::NoFrame);
		QObject::connect(&_geoItems, SIGNAL(addedTrackItem(Track, TrackItem*)),
						 this, SLOT(addTrack(Track,TrackItem*)));
		QObject::connect(&_geoItems, SIGNAL(addedRouteItem(Route,RouteItem*)),
						 this, SLOT(addRoute(Route,RouteItem*)));
		QObject::connect(&_geoItems, SIGNAL(cleared()),
						 this, SLOT(clear()));
	}

	virtual QString label() const = 0;
	virtual void setUnits(enum Units units) {GraphView::setUnits(units);}
	virtual void setGraphType(GraphType type) {GraphView::setGraphType(type);}
	virtual void setTimeType(enum TimeType type) {Q_UNUSED(type)}
	virtual void showTracks(bool show) {Q_UNUSED(show)}
	virtual void showRoutes(bool show) {Q_UNUSED(show)}

public slots:
	virtual void addTrack(const Track &track, TrackItem *item) = 0;
	virtual void addRoute(const Route &track, RouteItem *item) = 0;

private slots:
	virtual void clear() {GraphView::clear();}

private:
	GeoItems &_geoItems;
};

#endif // GRAPHTAB_H
