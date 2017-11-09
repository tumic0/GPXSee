#include "geoitems.h"
#include "trackitem.h"

GeoItems::GeoItems(Map *map, const Data *data, QObject *parent)
	: QObject(parent)
	, _map(map)
	, _showMap(true)
	, _showTracks(true)
	, _showRoutes(true)
	, _showWaypoints(true)
	, _showWaypointLabels(true)
	, _showPOI(true)
	, _showPOILabels(true)
	, _overlapPOIs(true)
	, _showRouteWaypoints(true)
	, _trackWidth(3)
	, _routeWidth(3)
	, _trackStyle(Qt::SolidLine)
	, _routeStyle(Qt::DashLine)
	, _waypointSize(8)
	, _poiSize(8)
	, _waypointColor(Qt::black)
	, _poiColor(Qt::black)
	, _digitalZoom(0)
{
	QObject::connect(data, SIGNAL(addedTrack(Track)),
					 this, SLOT(addTrack(Track)));
	QObject::connect(data, SIGNAL(addedRoute(Route)),
					 this, SLOT(addRoute(Route)));
	QObject::connect(data, SIGNAL(addedWaypoint(Waypoint)),
					 this, SLOT(addWaypoint(Waypoint)));
	QObject::connect(data, SIGNAL(cleared()),
					 this, SLOT(clear()));
}

GeoItems::~GeoItems()
{
	foreach (TrackItem* ti, _tracks) {
		delete ti;
	}
	foreach (RouteItem* ri, _routes) {
		delete ri;
	}
	foreach (WaypointItem *wi, _waypoints) {
		delete wi;
	}
}
void GeoItems::setDigitalZoom(int zoom)
{
	foreach (TrackItem* ti, _tracks) {
		ti->setDigitalZoom(zoom);
	}
	foreach (RouteItem* ri, _routes) {
		ri->setDigitalZoom(zoom);
	}
	foreach (WaypointItem *wi, _waypoints) {
		wi->setDigitalZoom(zoom);
	}

	emit digitalZoom(zoom);
}

void GeoItems::showTracks(bool show)
{
	_showTracks = show;

	foreach (TrackItem* ti, _tracks) {
		ti->setVisible(show);
	}
}

void GeoItems::showRoutes(bool show)
{
	_showRoutes = show;

	foreach (RouteItem* ri, _routes) {
		ri->setVisible(show);
	}
}

void GeoItems::showWaypoints(bool show)
{
	_showWaypoints = show;

	foreach (WaypointItem *wi, _waypoints) {
		wi->setVisible(show);
	}
}

void GeoItems::showWaypointLabels(bool show)
{
	_showWaypointLabels = show;

	foreach (WaypointItem *wi, _waypoints) {
		wi->showLabel(show);
	}

	foreach (RouteItem* ri, _routes) {
		ri->showWaypointLabels(show);
	}
}

void GeoItems::showRouteWaypoints(bool show)
{
	_showRouteWaypoints = show;

	foreach (RouteItem* ri, _routes) {
		ri->showWaypoints(show);
	}
}

void GeoItems::setPalette(const Palette &palette)
{
	_palette = palette;
	_palette.reset();

	foreach (TrackItem* ti, _tracks) {
		ti->setColor(_palette.nextColor());
	}

	foreach (RouteItem* ri, _routes) {
		ri->setColor(_palette.nextColor());
	}
}

void GeoItems::setMap(Map *map)
{
	_map = map;
	foreach (TrackItem* ti, _tracks) {
		ti->setMap(_map);
	}
	foreach (RouteItem* ri, _routes) {
		ri->setMap(_map);
	}
	foreach (WaypointItem *wi, _waypoints) {
		wi->setMap(_map);
	}
}

void GeoItems::setUnits(Units units)
{
	_units = units;
	foreach (TrackItem* ti, _tracks) {
		ti->setUnits(_units);
	}
	foreach (RouteItem* ri, _routes) {
		ri->setUnits(_units);
	}
	foreach (WaypointItem *wi, _waypoints) {
		wi->setUnits(_units);
	}

	emit unitsChanged(units);
}

void GeoItems::setTrackWidth(int width)
{
	_trackWidth = width;

	foreach (TrackItem* ti, _tracks) {
		ti->setWidth(width);
	}
}

void GeoItems::setRouteWidth(int width)
{
	_routeWidth = width;

	foreach (RouteItem* ri, _routes) {
		ri->setWidth(width);
	}
}

void GeoItems::setTrackStyle(Qt::PenStyle style)
{
	_trackStyle = style;

	foreach (TrackItem* ti, _tracks) {
		ti->setStyle(style);
	}
}

void GeoItems::setRouteStyle(Qt::PenStyle style)
{
	_routeStyle = style;

	foreach (RouteItem* ri, _routes) {
		ri->setStyle(style);
	}
}

void GeoItems::setWaypointSize(int size)
{
	_waypointSize = size;

	foreach (WaypointItem *wi, _waypoints) {
		wi->setSize(size);
	}
}

void GeoItems::setWaypointColor(const QColor &color)
{
	_waypointColor = color;

	foreach (WaypointItem *wi, _waypoints) {
		wi->setColor(color);
	}
}

void GeoItems::addTrack(const Track &t)
{
	if (!t.isNull()) {
		TrackItem *ti(new TrackItem(t, _map));
		_tracks.append(ti);
		ti->setColor(_palette.nextColor());
		ti->setWidth(_trackWidth);
		ti->setStyle(_trackStyle);
		ti->setUnits(_units);
		ti->setVisible(_showTracks);
		ti->setDigitalZoom(_digitalZoom);
		//addPOI(_poi->points(ti->path()));

		emit addedTrackItem(t, ti);
	}
}

void GeoItems::addRoute(const Route &r)
{
	if (!r.isNull()) {
		RouteItem *ri = new RouteItem(r, _map);
		_routes.append(ri);
		ri->setColor(_palette.nextColor());
		ri->setWidth(_routeWidth);
		ri->setStyle(_routeStyle);
		ri->setUnits(_units);
		ri->setVisible(_showRoutes);
		ri->showWaypoints(_showRouteWaypoints);
		ri->showWaypointLabels(_showWaypointLabels);
		ri->setDigitalZoom(_digitalZoom);
		//addPOI(_poi->points(ri->path()));

		emit addedRouteItem(r, ri);
	}
}

void GeoItems::addWaypoint(const Waypoint &w) {
	WaypointItem *wi = new WaypointItem(w, _map);
	_waypoints.append(wi);
	wi->setZValue(1);
	wi->setSize(_waypointSize);
	wi->setColor(_waypointColor);
	wi->showLabel(_showWaypointLabels);
	wi->setUnits(_units);
	wi->setVisible(_showWaypoints);
	wi->setDigitalZoom(_digitalZoom);
	//addPOI(_poi->points(w));

	emit addedWaypointItem(w, wi);
}

void GeoItems::clear()
{
	_tracks.clear();
	_routes.clear();
	_waypoints.clear();
	_palette.reset();
	emit cleared();
}

void GeoItems::addPOI(const QVector<Waypoint> &waypoints)
{
	foreach (const Waypoint &w, waypoints) {
		if (_pois.contains(SearchPointer<Waypoint>(&w)))
			continue;

		WaypointItem *pi = new WaypointItem(w, _map);
		pi->setZValue(1);
		pi->setSize(_poiSize);
		pi->setColor(_poiColor);
		pi->showLabel(_showPOILabels);
		pi->setVisible(_showPOI);
		pi->setDigitalZoom(_digitalZoom);
		//_scene->addItem(pi);

		_pois.insert(SearchPointer<Waypoint>(&(pi->waypoint())), pi);
	}
}

