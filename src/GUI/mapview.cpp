#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <QApplication>
#include <QPixmapCache>
#include <QScrollBar>
#include "data/poi.h"
#include "data/data.h"
#include "map/map.h"
#include "opengl.h"
#include "trackitem.h"
#include "routeitem.h"
#include "waypointitem.h"
#include "scaleitem.h"
#include "keys.h"
#include "mapview.h"


#define MAX_DIGITAL_ZOOM 2
#define MIN_DIGITAL_ZOOM -3
#define MARGIN           10
#define SCALE_OFFSET     7


MapView::MapView(Map *map, POI *poi, QWidget *parent)
  : QGraphicsView(parent)
{
	Q_ASSERT(map != 0);
	Q_ASSERT(poi != 0);

	_scene = new QGraphicsScene(this);
	setScene(_scene);
	setDragMode(QGraphicsView::ScrollHandDrag);
	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setRenderHint(QPainter::Antialiasing, true);
	setAcceptDrops(false);

	_mapScale = new ScaleItem();
	_mapScale->setZValue(2.0);
	_scene->addItem(_mapScale);

	_map = map;
	_map->load();
	connect(_map, SIGNAL(loaded()), this, SLOT(reloadMap()));

	_poi = poi;
	connect(_poi, SIGNAL(pointsChanged()), this, SLOT(updatePOI()));

	_units = Metric;
	_coordinatesFormat = DecimalDegrees;
	_opacity = 1.0;
	_backgroundColor = Qt::white;
	_markerColor = Qt::red;

	_showMap = true;
	_showTracks = true;
	_showRoutes = true;
	_showWaypoints = true;
	_showWaypointLabels = true;
	_showPOI = true;
	_showPOILabels = true;
	_overlapPOIs = true;
	_showRouteWaypoints = true;
	_trackWidth = 3;
	_routeWidth = 3;
	_trackStyle = Qt::SolidLine;
	_routeStyle = Qt::DashLine;
	_waypointSize = 8;
	_waypointColor = Qt::black;
	_poiSize = 8;
	_poiColor = Qt::black;

#ifdef ENABLE_HIDPI
	_ratio = 1.0;
#endif // ENABLE_HIDPI
	_opengl = false;
	_plot = false;
	_digitalZoom = 0;

	_res = _map->resolution(_map->bounds());
	_scene->setSceneRect(_map->bounds());

	centerOn(_scene->sceneRect().center());
}

void MapView::centerOn(const QPointF &pos)
{
	QGraphicsView::centerOn(pos);
	QRectF vr(mapToScene(viewport()->rect()).boundingRect());
	_res = _map->resolution(vr);
	_mapScale->setResolution(_res);
}

PathItem *MapView::addTrack(const Track &track)
{
	if (track.isNull()) {
		_palette.nextColor();
		return 0;
	}

	TrackItem *ti = new TrackItem(track, _map);
	_tracks.append(ti);
	_tr |= ti->path().boundingRect();
	ti->setColor(_palette.nextColor());
	ti->setWidth(_trackWidth);
	ti->setStyle(_trackStyle);
	ti->setUnits(_units);
	ti->setVisible(_showTracks);
	ti->setDigitalZoom(_digitalZoom);
	ti->setMarkerColor(_markerColor);
	_scene->addItem(ti);

	if (_showTracks)
		addPOI(_poi->points(ti->path()));

	return ti;
}

PathItem *MapView::addRoute(const Route &route)
{
	if (route.isNull()) {
		_palette.nextColor();
		return 0;
	}

	RouteItem *ri = new RouteItem(route, _map);
	_routes.append(ri);
	_rr |= ri->path().boundingRect();
	ri->setColor(_palette.nextColor());
	ri->setWidth(_routeWidth);
	ri->setStyle(_routeStyle);
	ri->setUnits(_units);
	ri->setCoordinatesFormat(_coordinatesFormat);
	ri->setVisible(_showRoutes);
	ri->showWaypoints(_showRouteWaypoints);
	ri->showWaypointLabels(_showWaypointLabels);
	ri->setDigitalZoom(_digitalZoom);
	ri->setMarkerColor(_markerColor);
	_scene->addItem(ri);

	if (_showRoutes)
		addPOI(_poi->points(ri->path()));

	return ri;
}

void MapView::addWaypoints(const QList<Waypoint> &waypoints)
{
	for (int i = 0; i < waypoints.count(); i++) {
		const Waypoint &w = waypoints.at(i);

		WaypointItem *wi = new WaypointItem(w, _map);
		_waypoints.append(wi);
		_wr = _wr.united(wi->waypoint().coordinates());
		wi->setZValue(1);
		wi->setSize(_waypointSize);
		wi->setColor(_waypointColor);
		wi->showLabel(_showWaypointLabels);
		wi->setToolTipFormat(_units, _coordinatesFormat);
		wi->setVisible(_showWaypoints);
		wi->setDigitalZoom(_digitalZoom);
		_scene->addItem(wi);

		if (_showWaypoints)
			addPOI(_poi->points(w));
	}
}

QList<PathItem *> MapView::loadData(const Data &data)
{
	QList<PathItem *> paths;
	int zoom = _map->zoom();

	for (int i = 0; i < data.tracks().count(); i++)
		paths.append(addTrack(*(data.tracks().at(i))));
	for (int i = 0; i < data.routes().count(); i++)
		paths.append(addRoute(*(data.routes().at(i))));
	addWaypoints(data.waypoints());

	if (_tracks.empty() && _routes.empty() && _waypoints.empty())
		return paths;

	if (fitMapZoom() != zoom)
		rescale();
	else
		updatePOIVisibility();

	centerOn(contentCenter());

	return paths;
}

int MapView::fitMapZoom() const
{
	RectC br = _tr | _rr | _wr;

	return _map->zoomFit(viewport()->size() - QSize(2*MARGIN, 2*MARGIN),
	  br.isNull() ? RectC(_map->xy2ll(_map->bounds().topLeft()),
	  _map->xy2ll(_map->bounds().bottomRight())) : br);
}

QPointF MapView::contentCenter() const
{
	RectC br = _tr | _rr | _wr;

	return br.isNull() ? sceneRect().center() : _map->ll2xy(br.center());
}

void MapView::updatePOIVisibility()
{
	QHash<SearchPointer<Waypoint>, WaypointItem*>::const_iterator it, jt;

	if (!_showPOI)
		return;

	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->show();

	if (!_overlapPOIs) {
		for (it = _pois.constBegin(); it != _pois.constEnd(); it++) {
			for (jt = _pois.constBegin(); jt != _pois.constEnd(); jt++) {
				if (it.value()->isVisible() && jt.value()->isVisible()
				  && it != jt && it.value()->collidesWithItem(jt.value()))
					jt.value()->hide();
			}
		}
	}
}

void MapView::rescale()
{
	_scene->setSceneRect(_map->bounds());
	reloadMap();

	for (int i = 0; i < _tracks.size(); i++)
		_tracks.at(i)->setMap(_map);
	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->setMap(_map);
	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->setMap(_map);

	QHash<SearchPointer<Waypoint>, WaypointItem*>::const_iterator it;
	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->setMap(_map);

	updatePOIVisibility();
}

void MapView::setPalette(const Palette &palette)
{
	_palette = palette;
	_palette.reset();

	for (int i = 0; i < _tracks.count(); i++)
		_tracks.at(i)->setColor(_palette.nextColor());
	for (int i = 0; i < _routes.count(); i++)
		_routes.at(i)->setColor(_palette.nextColor());
}

void MapView::setMap(Map *map)
{
	QRectF vr(mapToScene(viewport()->rect()).boundingRect()
	  .intersected(_map->bounds()));
	RectC cr(_map->xy2ll(vr.topLeft()), _map->xy2ll(vr.bottomRight()));

	_map->unload();
	disconnect(_map, SIGNAL(loaded()), this, SLOT(reloadMap()));

	_map = map;
	_map->load();
#ifdef ENABLE_HIDPI
	_map->setDevicePixelRatio(_ratio);
#endif // ENABLE_HIDPI
	connect(_map, SIGNAL(loaded()), this, SLOT(reloadMap()));

	digitalZoom(0);

	_map->zoomFit(viewport()->rect().size(), cr);
	_scene->setSceneRect(_map->bounds());

	for (int i = 0; i < _tracks.size(); i++)
		_tracks.at(i)->setMap(map);
	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->setMap(map);
	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->setMap(map);

	QHash<SearchPointer<Waypoint>, WaypointItem*>::const_iterator it;
	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->setMap(_map);
	updatePOIVisibility();

	QPointF nc = QRectF(_map->ll2xy(cr.topLeft()),
	  _map->ll2xy(cr.bottomRight())).center();
	centerOn(nc);

	reloadMap();
	QPixmapCache::clear();
}

void MapView::setPOI(POI *poi)
{
	disconnect(_poi, SIGNAL(pointsChanged()), this, SLOT(updatePOI()));
	connect(poi, SIGNAL(pointsChanged()), this, SLOT(updatePOI()));

	_poi = poi;

	updatePOI();
}

void MapView::updatePOI()
{
	QHash<SearchPointer<Waypoint>, WaypointItem*>::const_iterator it;

	for (it = _pois.constBegin(); it != _pois.constEnd(); it++) {
		_scene->removeItem(it.value());
		delete it.value();
	}
	_pois.clear();

	if (_showTracks)
		for (int i = 0; i < _tracks.size(); i++)
			addPOI(_poi->points(_tracks.at(i)->path()));
	if (_showRoutes)
		for (int i = 0; i < _routes.size(); i++)
			addPOI(_poi->points(_routes.at(i)->path()));
	if (_showWaypoints)
		for (int i = 0; i< _waypoints.size(); i++)
			addPOI(_poi->points(_waypoints.at(i)->waypoint()));

	updatePOIVisibility();
}

void MapView::addPOI(const QList<Waypoint> &waypoints)
{
	for (int i = 0; i < waypoints.size(); i++) {
		const Waypoint &w = waypoints.at(i);

		if (_pois.contains(SearchPointer<Waypoint>(&w)))
			continue;

		WaypointItem *pi = new WaypointItem(w, _map);
		pi->setZValue(1);
		pi->setSize(_poiSize);
		pi->setColor(_poiColor);
		pi->showLabel(_showPOILabels);
		pi->setVisible(_showPOI);
		pi->setDigitalZoom(_digitalZoom);
		pi->setToolTipFormat(_units, _coordinatesFormat);
		_scene->addItem(pi);

		_pois.insert(SearchPointer<Waypoint>(&(pi->waypoint())), pi);
	}
}

void MapView::setUnits(Units units)
{
	if (_units == units)
		return;

	_units = units;

	_mapScale->setUnits(_units);

	for (int i = 0; i < _tracks.count(); i++)
		_tracks[i]->setUnits(_units);
	for (int i = 0; i < _routes.count(); i++)
		_routes[i]->setUnits(_units);
	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->setToolTipFormat(_units, _coordinatesFormat);

	QHash<SearchPointer<Waypoint>, WaypointItem*>::const_iterator it;
	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->setToolTipFormat(_units, _coordinatesFormat);
}

void MapView::setCoordinatesFormat(CoordinatesFormat format)
{
	if (_coordinatesFormat == format)
		return;

	_coordinatesFormat = format;

	for (int i = 0; i < _waypoints.count(); i++)
		_waypoints.at(i)->setToolTipFormat(_units, _coordinatesFormat);
	for (int i = 0; i < _routes.count(); i++)
		_routes[i]->setCoordinatesFormat(_coordinatesFormat);

	QHash<SearchPointer<Waypoint>, WaypointItem*>::const_iterator it;
	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->setToolTipFormat(_units, _coordinatesFormat);
}

void MapView::clearMapCache()
{
	_map->clearCache();

	fitMapZoom();
	rescale();
	centerOn(contentCenter());
}

void MapView::digitalZoom(int zoom)
{
	QHash<SearchPointer<Waypoint>, WaypointItem*>::const_iterator it;

	if (zoom) {
		_digitalZoom += zoom;
		scale(pow(2, zoom), pow(2, zoom));
	} else {
		_digitalZoom = 0;
		resetTransform();
	}

	for (int i = 0; i < _tracks.size(); i++)
		_tracks.at(i)->setDigitalZoom(_digitalZoom);
	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->setDigitalZoom(_digitalZoom);
	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->setDigitalZoom(_digitalZoom);
	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->setDigitalZoom(_digitalZoom);

	_mapScale->setDigitalZoom(_digitalZoom);
}

void MapView::zoom(int zoom, const QPoint &pos)
{
	bool shift = QApplication::keyboardModifiers() & Qt::ShiftModifier;

	if (_digitalZoom) {
		if (((_digitalZoom > 0 && zoom > 0) && (!shift || _digitalZoom
		  >= MAX_DIGITAL_ZOOM)) || ((_digitalZoom < 0 && zoom < 0) && (!shift
		  || _digitalZoom <= MIN_DIGITAL_ZOOM)))
			return;

		digitalZoom(zoom);
	} else {
		Coordinates c = _map->xy2ll(mapToScene(pos));
		int oz = _map->zoom();
		int nz = (zoom > 0) ? _map->zoomIn() : _map->zoomOut();

		if (nz != oz) {
			rescale();
			centerOn(_map->ll2xy(c) - (pos - viewport()->rect().center()));
		} else {
			if (shift)
				digitalZoom(zoom);
		}
	}
}

void MapView::wheelEvent(QWheelEvent *event)
{
	static int deg = 0;

	deg += event->delta() / 8;
	if (qAbs(deg) < 15)
		return;
	deg = 0;

	zoom((event->delta() > 0) ? 1 : -1, event->pos());
}

void MapView::mouseDoubleClickEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton && event->button() != Qt::RightButton)
		return;

	zoom((event->button() == Qt::LeftButton) ? 1 : -1, event->pos());
}

void MapView::keyPressEvent(QKeyEvent *event)
{
	int z;

	QPoint pos = viewport()->rect().center();

	if (event->key() == ZOOM_IN)
		z = 1;
	else if (event->key() == ZOOM_OUT)
		z = -1;
	else if (_digitalZoom && event->key() == Qt::Key_Escape) {
		digitalZoom(0);
		return;
	} else {
		QGraphicsView::keyPressEvent(event);
		return;
	}

	zoom(z, pos);
}

void MapView::plot(QPainter *painter, const QRectF &target, qreal scale,
  bool hires)
{
	QRect orig, adj;
	qreal ratio, diff, q;
	QPointF origScene, origPos;
	int zoom;


	// Enter plot mode
	setUpdatesEnabled(false);
	_plot = true;
#ifdef ENABLE_HIDPI
	_map->setDevicePixelRatio(1.0);
#endif // ENABLE_HIDPI

	// Compute sizes & ratios
	orig = viewport()->rect();
	origPos = _mapScale->pos();

	if (orig.height() * (target.width() / target.height()) - orig.width() < 0) {
		ratio = target.height() / target.width();
		diff = (orig.width() * ratio) - orig.height();
		adj = orig.adjusted(0, -diff/2, 0, diff/2);
	} else {
		ratio = target.width() / target.height();
		diff = (orig.height() * ratio) - orig.width();
		adj = orig.adjusted(-diff/2, 0, diff/2, 0);
	}
	q = (target.width() / scale) / adj.width();

	// Adjust the view for printing
	if (hires) {
		zoom = _map->zoom();
		QRectF vr(mapToScene(orig).boundingRect());
		origScene = vr.center();

		QPointF s(painter->device()->logicalDpiX()
		  / (qreal)metric(QPaintDevice::PdmDpiX),
		  painter->device()->logicalDpiY()
		  / (qreal)metric(QPaintDevice::PdmDpiY));
		adj = QRect(0, 0, adj.width() * s.x(), adj.height() * s.y());
		_map->zoomFit(adj.size(), _tr | _rr | _wr);
		rescale();

		QPointF center = contentCenter();
		centerOn(center);
		adj.moveCenter(mapFromScene(center));

		_mapScale->setDigitalZoom(_digitalZoom - log2(s.x() / q));
		_mapScale->setPos(mapToScene(QPoint(adj.bottomRight() + QPoint(
		  -(SCALE_OFFSET + _mapScale->boundingRect().width()) * (s.x() / q),
		  -(SCALE_OFFSET + _mapScale->boundingRect().height()) * (s.x() / q)))));
	} else {
		_mapScale->setDigitalZoom(_digitalZoom - log2(1.0 / q));
		_mapScale->setPos(mapToScene(QPoint(adj.bottomRight() + QPoint(
		  -(SCALE_OFFSET + _mapScale->boundingRect().width()) / q ,
		  -(SCALE_OFFSET + _mapScale->boundingRect().height()) / q))));
	}

	// Print the view
	render(painter, target, adj);

	// Revert view changes to display mode
	if (hires) {
		_map->setZoom(zoom);
		rescale();
		centerOn(origScene);
	}
	_mapScale->setDigitalZoom(_digitalZoom);
	_mapScale->setPos(origPos);

	// Exit plot mode
#ifdef ENABLE_HIDPI
	_map->setDevicePixelRatio(_ratio);
#endif // ENABLE_HIDPI
	_plot = false;
	setUpdatesEnabled(true);
}

void MapView::clear()
{
	_pois.clear();
	_tracks.clear();
	_routes.clear();
	_waypoints.clear();

	_scene->removeItem(_mapScale);
	_scene->clear();
	_scene->addItem(_mapScale);

	_palette.reset();

	_tr = RectC();
	_rr = RectC();
	_wr = RectC();

	digitalZoom(0);

	// If not reset, causes huge redraw areas (and system memory exhaustion)
	resetCachedContent();
}

void MapView::showTracks(bool show)
{
	_showTracks = show;

	for (int i = 0; i < _tracks.count(); i++)
		_tracks.at(i)->setVisible(show);

	updatePOI();
}

void MapView::showRoutes(bool show)
{
	_showRoutes = show;

	for (int i = 0; i < _routes.count(); i++)
		_routes.at(i)->setVisible(show);

	updatePOI();
}

void MapView::showWaypoints(bool show)
{
	_showWaypoints = show;

	for (int i = 0; i < _waypoints.count(); i++)
		_waypoints.at(i)->setVisible(show);

	updatePOI();
}

void MapView::showWaypointLabels(bool show)
{
	_showWaypointLabels = show;

	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->showLabel(show);

	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->showWaypointLabels(show);
}

void MapView::showRouteWaypoints(bool show)
{
	_showRouteWaypoints = show;

	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->showWaypoints(show);
}

void MapView::showMap(bool show)
{
	_showMap = show;
	reloadMap();
}

void MapView::showPOI(bool show)
{
	_showPOI = show;

	QHash<SearchPointer<Waypoint>, WaypointItem*>::const_iterator it;
	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->setVisible(show);

	updatePOIVisibility();
}

void MapView::showPOILabels(bool show)
{
	_showPOILabels = show;

	QHash<SearchPointer<Waypoint>, WaypointItem*>::const_iterator it;
	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->showLabel(show);

	updatePOIVisibility();
}

void MapView::setPOIOverlap(bool overlap)
{
	_overlapPOIs = overlap;

	updatePOIVisibility();
}

void MapView::setTrackWidth(int width)
{
	_trackWidth = width;

	for (int i = 0; i < _tracks.count(); i++)
		_tracks.at(i)->setWidth(width);
}

void MapView::setRouteWidth(int width)
{
	_routeWidth = width;

	for (int i = 0; i < _routes.count(); i++)
		_routes.at(i)->setWidth(width);
}

void MapView::setTrackStyle(Qt::PenStyle style)
{
	_trackStyle = style;

	for (int i = 0; i < _tracks.count(); i++)
		_tracks.at(i)->setStyle(style);
}

void MapView::setRouteStyle(Qt::PenStyle style)
{
	_routeStyle = style;

	for (int i = 0; i < _routes.count(); i++)
		_routes.at(i)->setStyle(style);
}

void MapView::setWaypointSize(int size)
{
	_waypointSize = size;

	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->setSize(size);
}

void MapView::setWaypointColor(const QColor &color)
{
	_waypointColor = color;

	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->setColor(color);
}

void MapView::setPOISize(int size)
{
	QHash<SearchPointer<Waypoint>, WaypointItem*>::const_iterator it;

	_poiSize = size;

	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->setSize(size);
}

void MapView::setPOIColor(const QColor &color)
{
	QHash<SearchPointer<Waypoint>, WaypointItem*>::const_iterator it;

	_poiColor = color;

	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->setColor(color);
}

void MapView::setMapOpacity(int opacity)
{
	_opacity = opacity / 100.0;
	reloadMap();
}

void MapView::setBackgroundColor(const QColor &color)
{
	_backgroundColor = color;
	reloadMap();
}

void MapView::drawBackground(QPainter *painter, const QRectF &rect)
{
	painter->fillRect(rect, _backgroundColor);

	if (_showMap) {
		QRectF ir = rect.intersected(_map->bounds());
		Map::Flags flags = Map::NoFlags;

		if (_opacity < 1.0)
			painter->setOpacity(_opacity);

		if (_plot)
			flags = Map::Block;
		else if (_opengl)
			flags = Map::OpenGL;

		_map->draw(painter, ir, flags);
	}
}

void MapView::resizeEvent(QResizeEvent *event)
{
	QGraphicsView::resizeEvent(event);

	int zoom = _map->zoom();
	if (fitMapZoom() != zoom)
		rescale();

	centerOn(contentCenter());
}

void MapView::paintEvent(QPaintEvent *event)
{
	QPointF scenePos = mapToScene(rect().bottomRight() + QPoint(
	  -(SCALE_OFFSET + _mapScale->boundingRect().width()),
	  -(SCALE_OFFSET + _mapScale->boundingRect().height())));
	if (_mapScale->pos() != scenePos && !_plot)
		_mapScale->setPos(scenePos);

	QGraphicsView::paintEvent(event);
}

void MapView::scrollContentsBy(int dx, int dy)
{
	QGraphicsView::scrollContentsBy(dx, dy);

	QRectF sr(mapToScene(viewport()->rect()).boundingRect());
	qreal res = _map->resolution(sr);

	if (qMax(res, _res) / qMin(res, _res) > 1.1) {
		_mapScale->setResolution(res);
		_res = res;
	}
}

void MapView::useOpenGL(bool use)
{
	_opengl = use;

	if (use)
		setViewport(new OPENGL_WIDGET);
	else
		setViewport(new QWidget);
}

void MapView::useAntiAliasing(bool use)
{
	setRenderHint(QPainter::Antialiasing, use);
}

void MapView::setMarkerColor(const QColor &color)
{
	_markerColor = color;

	for (int i = 0; i < _tracks.size(); i++)
		_tracks.at(i)->setMarkerColor(color);
	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->setMarkerColor(color);
}

void MapView::reloadMap()
{
	_scene->invalidate();
}

void MapView::setDevicePixelRatio(qreal ratio)
{
#ifdef ENABLE_HIDPI
	if (_ratio == ratio)
		return;

	_ratio = ratio;

	QRectF vr(mapToScene(viewport()->rect()).boundingRect()
	  .intersected(_map->bounds()));
	RectC cr(_map->xy2ll(vr.topLeft()), _map->xy2ll(vr.bottomRight()));

	_map->setDevicePixelRatio(_ratio);
	_scene->setSceneRect(_map->bounds());

	for (int i = 0; i < _tracks.size(); i++)
		_tracks.at(i)->setMap(_map);
	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->setMap(_map);
	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->setMap(_map);

	QHash<SearchPointer<Waypoint>, WaypointItem*>::const_iterator it;
	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->setMap(_map);
	updatePOIVisibility();

	QPointF nc = QRectF(_map->ll2xy(cr.topLeft()),
	  _map->ll2xy(cr.bottomRight())).center();
	centerOn(nc);

	reloadMap();
#else // ENABLE_HIDPI
	Q_UNUSED(ratio);
#endif // ENABLE_HIDPI
}
