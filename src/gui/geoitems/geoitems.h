#ifndef GEOELEMENTS_H
#define GEOELEMENTS_H

#include <QObject>

#include "data.h"
#include "map.h"
#include "routeitem.h"
#include "trackitem.h"
#include "waypointitem.h"
#include "poi.h"
#include "palette.h"
#include "searchpointer.h"

class GeoItems : public QObject
{
	Q_OBJECT

public:
	GeoItems(Map *map, const Data *data, QObject *parent = 0);
	virtual ~GeoItems();

	int getDigitalZoom() {return _digitalZoom;}

	int trackCount() const {return _tracks.count();}
	int routeCount() const {return _routes.count();}
	int waypointCount() const {return _waypoints.count();}

signals:
	void digitalZoom(int zoom);
	void addedTrackItem(const Track& t, TrackItem *ti);
	void addedRouteItem(const Route& r, RouteItem *ri);
	void addedWaypointItem(const Waypoint &w, WaypointItem *wi);
	void cleared();
	void unitsChanged(enum Units units);

public slots:
	void setDigitalZoom(int zoom);
	void showTracks(bool show);
	void showRoutes(bool show);
	void showWaypoints(bool show);
	void showWaypointLabels(bool show);
	void showRouteWaypoints(bool show);
	void setPalette(const Palette &palette);
	void setMap(Map *map);
	void setUnits(enum Units units);
	void setTrackWidth(int width);
	void setRouteWidth(int width);
	void setTrackStyle(Qt::PenStyle style);
	void setRouteStyle(Qt::PenStyle style);
	void setWaypointSize(int size);
	void setWaypointColor(const QColor &color);

private slots:
	void addTrack(const Track &t);
	void addRoute(const Route &r);
	void addWaypoint(const Waypoint &r);
	void clear();

private:
	void addPOI(const QVector<Waypoint> &waypoints);

	Map *_map;
	QList<TrackItem*> _tracks;
	QList<RouteItem*> _routes;
	QList<WaypointItem*> _waypoints;

	// TEMP
	QHash<SearchPointer<Waypoint>, WaypointItem*> _pois;
	POI *_poi;

	// Styling
	Palette _palette;
	bool _showMap;
	bool _showTracks;
	bool _showRoutes;
	bool _showWaypoints;
	bool _showWaypointLabels;
	bool _showPOI;
	bool _showPOILabels;
	bool _overlapPOIs;
	bool _showRouteWaypoints;
	int _trackWidth;
	int _routeWidth;
	Qt::PenStyle _trackStyle;
	Qt::PenStyle _routeStyle;
	int _waypointSize;
	int _poiSize;
	QColor _waypointColor;
	QColor _poiColor;
	Units _units;
	int _digitalZoom;
};

#endif // GEOELEMENTS_H
