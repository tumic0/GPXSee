#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <QGestureEvent>
#include <QApplication>
#include <QScrollBar>
#include <QClipboard>
#include <QOpenGLWidget>
#include <QGeoPositionInfoSource>
#include "data/poi.h"
#include "data/data.h"
#include "map/map.h"
#include "map/pcs.h"
#include "trackitem.h"
#include "routeitem.h"
#include "waypointitem.h"
#include "areaitem.h"
#include "scaleitem.h"
#include "coordinatesitem.h"
#include "mapitem.h"
#include "keys.h"
#include "mapaction.h"
#include "markerinfoitem.h"
#include "crosshairitem.h"
#include "motioninfoitem.h"
#include "mapview.h"


#define MAX_DIGITAL_ZOOM 2
#define MIN_DIGITAL_ZOOM -3
#define MARGIN           10
#define SCALE_OFFSET     7
#define COORDINATES_OFFSET SCALE_OFFSET


MapView::MapView(Map *map, POI *poi, QWidget *parent) : QGraphicsView(parent)
{
	Q_ASSERT(map != 0);
	Q_ASSERT(poi != 0);

	_scene = new GraphicsScene(this);
	setScene(_scene);
	setDragMode(QGraphicsView::ScrollHandDrag);
	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
	setResizeAnchor(QGraphicsView::AnchorViewCenter);
	setAcceptDrops(false);
	viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
	grabGesture(Qt::PinchGesture);

	_mapScale = new ScaleItem();
	_mapScale->setZValue(2.0);
	_scene->addItem(_mapScale);
	_cursorCoordinates = new CoordinatesItem();
	_cursorCoordinates->setZValue(2.0);
	_cursorCoordinates->setVisible(false);
	_scene->addItem(_cursorCoordinates);

	_deviceRatio = devicePixelRatioF();
	_outputProjection = PCS::pcs(3857);
	_inputProjection = GCS::gcs(4326);
	_hidpi = true;
	_hillShading = true;
	_layers = Layer::Raster | Layer::Vector;
	_map = map;
	_map->load(_inputProjection, _outputProjection, _deviceRatio, _hidpi);
	connect(_map, &Map::tilesLoaded, this, &MapView::reloadMap);

	_poi = poi;
	connect(_poi, &POI::pointsChanged, this, &MapView::updatePOI);

	_positionSource = 0;
	_crosshair = new CrosshairItem();
	_crosshair->setZValue(2.0);
	_crosshair->setVisible(false);
	_scene->addItem(_crosshair);

	_positionCoordinates = new CoordinatesItem();
	_positionCoordinates->setZValue(2.0);
	_positionCoordinates->setVisible(false);
	_scene->addItem(_positionCoordinates);

	_motionInfo = new MotionInfoItem();
	_motionInfo->setZValue(2.0);
	_motionInfo->setVisible(false);
	_scene->addItem(_motionInfo);

	_mapOpacity = 1.0;
	_backgroundColor = Qt::white;
	_markerColor = Qt::red;

	_showMap = false;
	_showTracks = false;
	_showRoutes = false;
	_showAreas = false;
	_showWaypoints = false;
	_showWaypointLabels = false;
	_showWaypointIcons = false;
	_showPOI = false;
	_showPOILabels = false;
	_showPOIIcons = false;
	_overlapPOIs = false;
	_showRouteWaypoints = false;
	_showMarkers = false;
	_markerInfoType = MarkerInfoItem::None;
	_showPathTicks = false;
	_trackWidth = 3;
	_routeWidth = 3;
	_trackStyle = Qt::SolidLine;
	_routeStyle = Qt::DashLine;
	_waypointSize = 8;
	_waypointColor = Qt::black;
	_poiSize = 8;
	_poiColor = Qt::black;
	_followPosition = false;
	_showPosition = false;
	_showPositionCoordinates = false;
	_showMotionInfo = false;
	_infoBackground = false;

	_opengl = false;
	_plot = false;
	_digitalZoom = 0;
	_pinchZoom = 0;
	_wheelDelta = 0;

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
	_cursorCoordinates->setCoordinates(Coordinates());
}

PathItem *MapView::addTrack(const Track &track)
{
	if (!track.isValid()) {
		skipColor();
		return 0;
	}

	TrackItem *ti = new TrackItem(track, _map);
	_tracks.append(ti);
	_tr |= ti->path().boundingRect();
	ti->setColor(_palette.nextColor());
	ti->setWidth(_trackWidth);
	ti->setPenStyle(_trackStyle);
	ti->setVisible(_showTracks);
	ti->setDigitalZoom(_digitalZoom);
	ti->setMarkerColor(_markerColor);
	ti->setMarkerBackgroundColor(_backgroundColor);
	ti->drawMarkerBackground(_infoBackground);
	ti->showMarker(_showMarkers);
	ti->showMarkerInfo(_markerInfoType);
	ti->showTicks(_showPathTicks);
	_scene->addItem(ti);

	if (_showTracks)
		addPOI(_poi->points(ti->path()));

	return ti;
}

PathItem *MapView::addRoute(const Route &route)
{
	if (!route.isValid()) {
		skipColor();
		return 0;
	}

	RouteItem *ri = new RouteItem(route, _map);
	_routes.append(ri);
	_rr |= ri->path().boundingRect();
	ri->setColor(_palette.nextColor());
	ri->setWidth(_routeWidth);
	ri->setPenStyle(_routeStyle);
	ri->setVisible(_showRoutes);
	ri->showWaypoints(_showRouteWaypoints);
	ri->showWaypointLabels(_showWaypointLabels);
	ri->showWaypointIcons(_showWaypointIcons);
	ri->setDigitalZoom(_digitalZoom);
	ri->setMarkerColor(_markerColor);
	ri->setMarkerBackgroundColor(_backgroundColor);
	ri->drawMarkerBackground(_infoBackground);
	ri->showMarker(_showMarkers);
	ri->showMarkerInfo(_markerInfoType);
	ri->showTicks(_showPathTicks);
	_scene->addItem(ri);

	if (_showRoutes)
		addPOI(_poi->points(ri->path()));

	return ri;
}

void MapView::addArea(const Area &area)
{
	if (!area.isValid()) {
		skipColor();
		return;
	}

	AreaItem *ai = new AreaItem(area, _map);
	ai->setColor(_palette.nextColor());
	ai->setWidth(_areaWidth);
	ai->setPenStyle(_areaStyle);
	ai->setOpacity(_areaOpacity);
	ai->setDigitalZoom(_digitalZoom);
	ai->setVisible(_showAreas);
	ai->setZValue(-area.boundingRect().area());

	_scene->addItem(ai);
	_ar |= ai->bounds();
	_areas.append(ai);

	if (_showAreas)
		addPOI(_poi->points(ai->bounds()));
}

void MapView::addWaypoints(const QVector<Waypoint> &waypoints)
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
		wi->showIcon(_showWaypointIcons);
		wi->setVisible(_showWaypoints);
		wi->setDigitalZoom(_digitalZoom);
		_scene->addItem(wi);

		if (_showWaypoints)
			addPOI(_poi->points(w));
	}
}

MapItem *MapView::addMap(MapAction *map)
{
	MapItem *mi = new MapItem(map, _map);
	mi->setColor(_palette.nextColor());
	mi->setWidth(_areaWidth);
	mi->setPenStyle(_areaStyle);
	mi->setOpacity(_areaOpacity);
	mi->setDigitalZoom(_digitalZoom);
	mi->setVisible(_showAreas);
	mi->setZValue(-mi->bounds().area());

	_scene->addItem(mi);
	_ar |= mi->bounds();
	_areas.append(mi);

	if (_showAreas)
		addPOI(_poi->points(mi->bounds()));

	return mi;
}

QList<PathItem *> MapView::loadData(const Data &data)
{
	QList<PathItem *> paths;
	int zoom = _map->zoom();

	for (int i = 0; i < data.areas().count(); i++)
		addArea(data.areas().at(i));
	for (int i = 0; i < data.tracks().count(); i++)
		paths.append(addTrack(data.tracks().at(i)));
	for (int i = 0; i < data.routes().count(); i++)
		paths.append(addRoute(data.routes().at(i)));
	addWaypoints(data.waypoints());

	if (_tracks.empty() && _routes.empty() && _waypoints.empty()
	  && _areas.empty())
		return paths;

	if (fitMapZoom() != zoom)
		rescale();
	else
		updatePOIVisibility();

	centerOn(contentCenter());

	return paths;
}

void MapView::loadMaps(const QList<MapAction *> &maps)
{
	int zoom = _map->zoom();

	for (int i = 0; i < maps.size(); i++)
		addMap(maps.at(i));

	if (fitMapZoom() != zoom)
		rescale();
	else
		updatePOIVisibility();

	centerOn(contentCenter());
}

void MapView::loadDEMs(const QList<Area> &dems)
{
	int zoom = _map->zoom();

	for (int i = 0; i < dems.size(); i++)
		addArea(dems.at(i));

	if (fitMapZoom() != zoom)
		rescale();
	else
		updatePOIVisibility();

	centerOn(contentCenter());
}

int MapView::fitMapZoom() const
{
	RectC br = _tr | _rr | _wr | _ar;

	return _map->zoomFit(viewport()->size() - QSize(2*MARGIN, 2*MARGIN),
	  br.isNull() ? _map->llBounds() : br);
}

QPointF MapView::contentCenter() const
{
	RectC br = _tr | _rr | _wr | _ar;

	return br.isNull() ? sceneRect().center() : _map->ll2xy(br.center());
}

void MapView::updatePOIVisibility()
{
	if (!_showPOI)
		return;

	for (POIHash::const_iterator it = _pois.constBegin();
	  it != _pois.constEnd(); it++)
		it.value()->show();

	if (!_overlapPOIs) {
		for (POIHash::const_iterator it = _pois.constBegin();
		  it != _pois.constEnd(); it++) {
			for (POIHash::const_iterator jt = _pois.constBegin();
			  jt != _pois.constEnd(); jt++) {
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
	for (int i = 0; i < _areas.size(); i++)
		_areas.at(i)->setMap(_map);
	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->setMap(_map);

	for (POIHash::const_iterator it = _pois.constBegin();
	  it != _pois.constEnd(); it++)
		it.value()->setMap(_map);

	_crosshair->setMap(_map);

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
	for (int i = 0; i < _areas.count(); i++)
		_areas.at(i)->setColor(_palette.nextColor());
}

void MapView::setMap(Map *map)
{
	QRectF vr(mapToScene(viewport()->rect()).boundingRect()
	  .intersected(_map->bounds()));
	RectC cr(_map->xy2ll(vr.topLeft()), _map->xy2ll(vr.bottomRight()));

	disconnect(_map, &Map::tilesLoaded, this, &MapView::reloadMap);
	_map->unload();

	_map = map;
	_map->load(_inputProjection, _outputProjection, _deviceRatio, _hidpi);
	connect(_map, &Map::tilesLoaded, this, &MapView::reloadMap);

	digitalZoom(0);

	_map->zoomFit(viewport()->rect().size(), cr);

	rescale();

	QPointF nc = QRectF(_map->ll2xy(cr.topLeft()),
	  _map->ll2xy(cr.bottomRight())).center();
	centerOn(nc);

	reloadMap();
}

void MapView::setPOI(POI *poi)
{
	disconnect(_poi, &POI::pointsChanged, this, &MapView::updatePOI);
	connect(poi, &POI::pointsChanged, this, &MapView::updatePOI);

	_poi = poi;

	updatePOI();
}

void MapView::setPositionSource(QGeoPositionInfoSource *source)
{
	if (_positionSource)
		disconnect(_positionSource, &QGeoPositionInfoSource::positionUpdated,
		  this, &MapView::updatePosition);
	if (source)
		connect(source, &QGeoPositionInfoSource::positionUpdated, this,
		  &MapView::updatePosition);

	_positionSource = source;

	showPosition(_showPosition);
}

void MapView::setGraph(int index)
{
	for (int i = 0; i < _tracks.size(); i++)
		_tracks.at(i)->setGraph(index);
	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->setGraph(index);
}

void MapView::updatePOI()
{
	for (POIHash::const_iterator it = _pois.constBegin();
	  it != _pois.constEnd(); it++)
		_scene->removeItem(it.value());
	qDeleteAll(_pois);
	_pois.clear();

	if (_showTracks)
		for (int i = 0; i < _tracks.size(); i++)
			addPOI(_poi->points(_tracks.at(i)->path()));
	if (_showRoutes)
		for (int i = 0; i < _routes.size(); i++)
			addPOI(_poi->points(_routes.at(i)->path()));
	if (_showAreas)
		for (int i = 0; i < _areas.size(); i++)
			addPOI(_poi->points(_areas.at(i)->bounds()));
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
		pi->showIcon(_showPOIIcons);
		pi->setVisible(_showPOI);
		pi->setDigitalZoom(_digitalZoom);
		_scene->addItem(pi);

		_pois.insert(SearchPointer<Waypoint>(&(pi->waypoint())), pi);
	}
}

void MapView::setUnits(Units units)
{
	WaypointItem::setUnits(units);
	PathItem::setUnits(units);

	for (int i = 0; i < _tracks.count(); i++)
		_tracks.at(i)->updateTicks();
	for (int i = 0; i < _routes.count(); i++)
		_routes.at(i)->updateTicks();

	_mapScale->setUnits(units);
	_cursorCoordinates->setUnits(units);
	_positionCoordinates->setUnits(units);
	_motionInfo->setUnits(units);
}

void MapView::setCoordinatesFormat(CoordinatesFormat format)
{
	WaypointItem::setCoordinatesFormat(format);
	MarkerInfoItem::setCoordinatesFormat(format);

	for (int i = 0; i < _tracks.count(); i++)
		_tracks.at(i)->updateMarkerInfo();
	for (int i = 0; i < _routes.count(); i++)
		_routes.at(i)->updateMarkerInfo();

	_cursorCoordinates->setFormat(format);
	_positionCoordinates->setFormat(format);
}

void MapView::setTimeZone(const QTimeZone &zone)
{
	WaypointItem::setTimeZone(zone);
	PathItem::setTimeZone(zone);

	for (int i = 0; i < _tracks.count(); i++)
		_tracks.at(i)->updateMarkerInfo();
	for (int i = 0; i < _routes.count(); i++)
		_routes.at(i)->updateMarkerInfo();
}

void MapView::clearMapCache()
{
	_map->clearCache();
	reloadMap();
}

void MapView::digitalZoom(int zoom)
{
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
	for (int i = 0; i < _areas.size(); i++)
		_areas.at(i)->setDigitalZoom(_digitalZoom);
	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->setDigitalZoom(_digitalZoom);
	for (POIHash::const_iterator it = _pois.constBegin();
	  it != _pois.constEnd(); it++)
		it.value()->setDigitalZoom(_digitalZoom);

	_mapScale->setDigitalZoom(_digitalZoom);
	_cursorCoordinates->setDigitalZoom(_digitalZoom);
	_positionCoordinates->setDigitalZoom(_digitalZoom);
	_motionInfo->setDigitalZoom(_digitalZoom);
	_crosshair->setDigitalZoom(_digitalZoom);
}

void MapView::zoom(int zoom, const QPoint &pos, bool shift)
{
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

void MapView::pinchGesture(QPinchGesture *gesture)
{
	QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
	qreal scaleFactor = gesture->totalScaleFactor();

	if (changeFlags & QPinchGesture::ScaleFactorChanged) {
		int z = 0;

		for (qreal sc = scaleFactor; sc > 1.25; sc *= 0.8)
			z += 1;
		for (qreal sc = scaleFactor; sc < 0.8; sc *= 1.25)
			z -= 1;

		if (_pinchZoom != z) {
			zoom(z - _pinchZoom, gesture->centerPoint().toPoint(), false);
			_pinchZoom = z;
		}
	}
	if (gesture->state() == Qt::GestureFinished)
		_pinchZoom = 0;
}

void MapView::wheelEvent(QWheelEvent *event)
{
	bool shift = (event->modifiers() & MODIFIER) ? true : false;
	// Shift inverts the wheel axis on OS X, so use scrolling in both axes for
	// the zoom.
	int delta = event->angleDelta().y()
	  ? event->angleDelta().y() : event->angleDelta().x();

	_wheelDelta += delta;
	if (qAbs(_wheelDelta) < (15 * 8))
		return;
	_wheelDelta = _wheelDelta % (15 * 8);

	zoom((delta > 0) ? 1 : -1, event->position().toPoint(), shift);

	/* Do not call QGraphicsView::wheelEvent() here as this would shift the
	   view ! */
}

void MapView::keyPressEvent(QKeyEvent *event)
{
	int z;
	bool shift = (event->modifiers() & MODIFIER) ? true : false;
	QPoint pos = viewport()->rect().center();

	if (event->key() == ZOOM_IN)
		z = 1;
	else if (event->key() == ZOOM_OUT)
		z = -1;
	else if (_digitalZoom && event->key() == Qt::Key_Escape) {
		digitalZoom(0);
		return;
	} else  {
		if (event->key() == MODIFIER_KEY) {
			_cursor = viewport()->cursor();
			viewport()->setCursor(Qt::CrossCursor);
		}

		QGraphicsView::keyPressEvent(event);
		return;
	}

	zoom(z, pos, shift);
}

void MapView::keyReleaseEvent(QKeyEvent *event)
{
	if (event->key() == MODIFIER_KEY
	  && viewport()->cursor().shape() == Qt::CrossCursor)
		viewport()->setCursor(_cursor);

	QGraphicsView::keyReleaseEvent(event);
}

void MapView::plot(QPainter *painter, const QRectF &target, qreal scale,
  Flags flags)
{
	QRect orig;
	qreal ratio, diff, q, p;
	QPointF scenePos, scalePos, posPos, motionPos;
	bool hidpi = _hidpi && _deviceRatio > 1.0;
	int zoom;


	// Enter plot mode
	setUpdatesEnabled(false);
	_plot = true;

	// Compute sizes & ratios
	orig = viewport()->rect();
	QRectF adj(orig);
	scalePos = _mapScale->pos();
	posPos = _positionCoordinates->pos();
	motionPos = _motionInfo->pos();

	if (orig.height() * (target.width() / target.height()) - orig.width() < 0) {
		ratio = target.height() / target.width();
		diff = (orig.width() * ratio) - orig.height();
		adj.adjust(0, -diff/2.0, 0, diff/2.0);
	} else {
		ratio = target.width() / target.height();
		diff = (orig.height() * ratio) - orig.width();
		adj.adjust(-diff/2.0, 0, diff/2.0, 0);
	}

	// Expand the view if plotting into a bitmap
	if (hidpi)
		setHidpi(false);

	if (flags & Expand) {
		qreal xdiff = (target.width() - adj.width()) / 2.0;
		qreal ydiff = (target.height() - adj.height()) / 2.0;
		adj.adjust(-xdiff, -ydiff, xdiff, ydiff);
		q = 1.0;
	} else
		q = (target.width() / scale) / adj.width();

	// Adjust the view for printing
	if (flags & HiRes) {
		zoom = _map->zoom();
		QRectF vr(mapToScene(orig).boundingRect());
		scenePos = vr.center();

		QPointF s(painter->device()->logicalDpiX()
		  / (qreal)metric(QPaintDevice::PdmDpiX),
		  painter->device()->logicalDpiY()
		  / (qreal)metric(QPaintDevice::PdmDpiY));
		adj = QRectF(0, 0, adj.width() * s.x(), adj.height() * s.y());
		_map->zoomFit(adj.size().toSize(), _tr | _rr | _wr | _ar);
		rescale();

		QPointF center = contentCenter();
		centerOn(center);
		adj.moveCenter(mapFromScene(center));

		p = s.x() / q;
	} else
		p = 1 / q;

	_mapScale->setDigitalZoom(_digitalZoom - log2(p));
	_mapScale->setPos(mapToScene(adj.bottomRight().toPoint() + QPoint(
	  -(SCALE_OFFSET + _mapScale->boundingRect().width()) * p,
	  -(SCALE_OFFSET + _mapScale->boundingRect().height()) * p)));
	_positionCoordinates->setDigitalZoom(_digitalZoom - log2(p));
	_positionCoordinates->setPos(mapToScene(adj.topLeft().toPoint() + QPoint(
	  COORDINATES_OFFSET * p,
	  (COORDINATES_OFFSET + _positionCoordinates->boundingRect().height()) * p)));
	_motionInfo->setDigitalZoom(_digitalZoom - log2(p));
	_motionInfo->setPos(mapToScene(adj.topRight().toPoint() + QPoint(
	  (-COORDINATES_OFFSET - _motionInfo->boundingRect().width()) * p,
	  (COORDINATES_OFFSET + _motionInfo->boundingRect().height()) * p)));

	// Print the view
	render(painter, target, adj.toRect());

	// Revert view changes to display mode
	if (flags & HiRes) {
		_map->setZoom(zoom);
		rescale();
		centerOn(scenePos);
	}

	if (hidpi)
		setHidpi(true);

	_mapScale->setDigitalZoom(_digitalZoom);
	_mapScale->setPos(scalePos);
	_positionCoordinates->setDigitalZoom(_digitalZoom);
	_positionCoordinates->setPos(posPos);
	_motionInfo->setDigitalZoom(_digitalZoom);
	_motionInfo->setPos(motionPos);

	// Exit plot mode
	_plot = false;
	setUpdatesEnabled(true);
}

void MapView::clear()
{
	_pois.clear();
	_tracks.clear();
	_routes.clear();
	_areas.clear();
	_waypoints.clear();

	_scene->removeItem(_mapScale);
	_scene->removeItem(_cursorCoordinates);
	_scene->removeItem(_positionCoordinates);
	_scene->removeItem(_crosshair);
	_scene->removeItem(_motionInfo);
	_scene->clear();
	_scene->addItem(_mapScale);
	_scene->addItem(_cursorCoordinates);
	_scene->addItem(_positionCoordinates);
	_scene->addItem(_crosshair);
	_scene->addItem(_motionInfo);

	_palette.reset();

	_tr = RectC();
	_rr = RectC();
	_wr = RectC();
	_ar = RectC();

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

void MapView::showAreas(bool show)
{
	_showAreas = show;

	for (int i = 0; i < _areas.count(); i++)
		_areas.at(i)->setVisible(show);

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

void MapView::showWaypointIcons(bool show)
{
	_showWaypointIcons = show;

	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->showIcon(show);
	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->showWaypointIcons(show);
}

void MapView::showRouteWaypoints(bool show)
{
	_showRouteWaypoints = show;

	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->showWaypoints(show);
}

void MapView::showMarkers(bool show)
{
	_showMarkers = show;

	for (int i = 0; i < _tracks.size(); i++)
		_tracks.at(i)->showMarker(show);
	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->showMarker(show);
}

void MapView::showMarkerInfo(MarkerInfoItem::Type type)
{
	_markerInfoType = type;

	for (int i = 0; i < _tracks.size(); i++)
		_tracks.at(i)->showMarkerInfo(type);
	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->showMarkerInfo(type);
}

void MapView::showTicks(bool show)
{
	_showPathTicks = show;
	for (int i = 0; i < _tracks.size(); i++)
		_tracks.at(i)->showTicks(show);
	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->showTicks(show);
}

void MapView::showMap(bool show)
{
	_showMap = show;
	reloadMap();
}

void MapView::showPOI(bool show)
{
	_showPOI = show;

	for (POIHash::const_iterator it = _pois.constBegin();
	  it != _pois.constEnd(); it++)
		it.value()->setVisible(show);

	updatePOIVisibility();
}

void MapView::showPOILabels(bool show)
{
	_showPOILabels = show;

	for (POIHash::const_iterator it = _pois.constBegin();
	  it != _pois.constEnd(); it++)
		it.value()->showLabel(show);

	updatePOIVisibility();
}

void MapView::showPOIIcons(bool show)
{
	_showPOIIcons = show;

	for (POIHash::const_iterator it = _pois.constBegin();
	  it != _pois.constEnd(); it++)
		it.value()->showIcon(show);

	updatePOIVisibility();
}

void MapView::showCursorCoordinates(bool show)
{
	_cursorCoordinates->setVisible(show);
	setMouseTracking(show);
	_scene->invalidate();
}

void MapView::showPositionCoordinates(bool show)
{
	_showPositionCoordinates = show;

	if (_crosshair->isVisible())
		_positionCoordinates->setVisible(show);

	_scene->invalidate();
}

void MapView::showMotionInfo(bool show)
{
	_showMotionInfo = show;

	if (_crosshair->isVisible())
		_motionInfo->setVisible(show);

	_scene->invalidate();
}

void MapView::showOverlappedPOIs(bool show)
{
	_overlapPOIs = show;

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

void MapView::setAreaWidth(int width)
{
	_areaWidth = width;

	for (int i = 0; i < _areas.count(); i++)
		_areas.at(i)->setWidth(width);
}

void MapView::setTrackStyle(Qt::PenStyle style)
{
	_trackStyle = style;

	for (int i = 0; i < _tracks.count(); i++)
		_tracks.at(i)->setPenStyle(style);
}

void MapView::setRouteStyle(Qt::PenStyle style)
{
	_routeStyle = style;

	for (int i = 0; i < _routes.count(); i++)
		_routes.at(i)->setPenStyle(style);
}

void MapView::setAreaStyle(Qt::PenStyle style)
{
	_areaStyle = style;

	for (int i = 0; i < _areas.count(); i++)
		_areas.at(i)->setPenStyle(style);
}

void MapView::setAreaOpacity(int opacity)
{
	_areaOpacity = opacity / 100.0;

	for (int i = 0; i < _areas.count(); i++)
		_areas.at(i)->setOpacity(_areaOpacity);
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
	_poiSize = size;

	for (POIHash::const_iterator it = _pois.constBegin();
	  it != _pois.constEnd(); it++)
		it.value()->setSize(size);
}

void MapView::setPOIColor(const QColor &color)
{
	_poiColor = color;

	for (POIHash::const_iterator it = _pois.constBegin();
	  it != _pois.constEnd(); it++)
		it.value()->setColor(color);
}

void MapView::setMapOpacity(int opacity)
{
	_mapOpacity = opacity / 100.0;
	reloadMap();
}

void MapView::setBackgroundColor(const QColor &color)
{
	_backgroundColor = color;
	_cursorCoordinates->setBackgroundColor(color);
	_positionCoordinates->setBackgroundColor(color);
	_motionInfo->setBackgroundColor(color);

	for (int i = 0; i < _tracks.size(); i++)
		_tracks.at(i)->setMarkerBackgroundColor(color);
	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->setMarkerBackgroundColor(color);

	reloadMap();
}

void MapView::drawBackground(QPainter *painter, const QRectF &rect)
{
	painter->fillRect(rect, _backgroundColor);

	if (_showMap) {
		QRectF ir = rect.intersected(_map->bounds());
		Map::Flags flags = Map::NoFlags;

		if (_mapOpacity < 1.0)
			painter->setOpacity(_mapOpacity);

		if (_plot)
			flags = Map::Block;
		else if (_opengl)
			flags = Map::OpenGL;
		if (_hillShading)
			flags |= Map::HillShading;
		if (_layers & Layer::Raster)
			flags |= Map::Rasters;
		if (_layers & Layer::Vector)
			flags |= Map::Vectors;

		_map->draw(painter, ir, flags);
	}
}

void MapView::paintEvent(QPaintEvent *event)
{
	if (!_plot) {
		QPointF scaleScenePos = mapToScene(rect().bottomRight() + QPoint(
		  -(SCALE_OFFSET + _mapScale->boundingRect().width()),
		  -(SCALE_OFFSET + _mapScale->boundingRect().height())));
		if (_mapScale->pos() != scaleScenePos)
			_mapScale->setPos(scaleScenePos);

		if (_cursorCoordinates->isVisible()) {
			QPointF coordinatesScenePos = mapToScene(rect().bottomLeft()
			  + QPoint(COORDINATES_OFFSET, -COORDINATES_OFFSET));
			if (_cursorCoordinates->pos() != coordinatesScenePos)
				_cursorCoordinates->setPos(coordinatesScenePos);
		}

		if (_positionCoordinates->isVisible()) {
			QPointF coordinatesScenePos = mapToScene(rect().topLeft()
			  + QPoint(COORDINATES_OFFSET, COORDINATES_OFFSET
			  + _positionCoordinates->boundingRect().height()));
			if (_positionCoordinates->pos() != coordinatesScenePos)
				_positionCoordinates->setPos(coordinatesScenePos);
		}

		if (_motionInfo->isVisible()) {
			QPointF coordinatesScenePos = mapToScene(rect().topRight()
			  + QPoint(-COORDINATES_OFFSET - _motionInfo->boundingRect().width(),
			  COORDINATES_OFFSET + _motionInfo->boundingRect().height()));
			if (_motionInfo->pos() != coordinatesScenePos)
				_motionInfo->setPos(coordinatesScenePos);
		}
	}

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

void MapView::leaveEvent(QEvent *event)
{
	_cursorCoordinates->setCoordinates(Coordinates());
	QGraphicsView::leaveEvent(event);
}

void MapView::mouseMoveEvent(QMouseEvent *event)
{
	if (_cursorCoordinates->isVisible()) {
		Coordinates c(_map->xy2ll(mapToScene(event->pos())));
		_cursorCoordinates->setCoordinates(c, _map->elevation(c));
	}

	QGraphicsView::mouseMoveEvent(event);
}


void MapView::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		if (event->modifiers() & MODIFIER)
			QApplication::clipboard()->setText(Format::coordinates(_map->xy2ll(
			  mapToScene(event->pos())), _cursorCoordinates->format()));
#ifdef Q_OS_ANDROID
		else
			emit clicked(event->pos());
#endif // Q_OS_ANDROID
	}

	QGraphicsView::mousePressEvent(event);
}

void MapView::mouseDoubleClickEvent(QMouseEvent *event)
{
	bool shift = (event->modifiers() & MODIFIER) ? true : false;

	if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton)
		zoom((event->button() == Qt::LeftButton) ? 1 : -1, event->pos(), shift);

	QGraphicsView::mouseDoubleClickEvent(event);
}

bool MapView::event(QEvent *event)
{
	if (event->type() == QEvent::Gesture)
		return gestureEvent(static_cast<QGestureEvent*>(event));

	return QGraphicsView::event(event);
}

bool MapView::gestureEvent(QGestureEvent *event)
{
	if (QGesture *pinch = event->gesture(Qt::PinchGesture))
		pinchGesture(static_cast<QPinchGesture *>(pinch));

	return true;
}

void MapView::useOpenGL(bool use)
{
	if (_opengl == use)
		return;

	_opengl = use;

	if (use)
		setViewport(new QOpenGLWidget);
	else
		setViewport(new QWidget);
}

void MapView::useAntiAliasing(bool use)
{
	setRenderHint(QPainter::Antialiasing, use);
}

void MapView::drawHillShading(bool draw)
{
	_hillShading = draw;

	setMap(_map);
}

void MapView::selectLayers(Layers layers)
{
	_layers = layers;

	setMap(_map);
}

void MapView::useStyles(bool use)
{
	GraphicsItem::useStyle(use);

	for (int i = 0; i < _tracks.size(); i++)
		_tracks.at(i)->updateStyle();
	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->updateStyle();
	for (int i = 0; i < _areas.size(); i++)
		_areas.at(i)->updateStyle();
	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->updateStyle();
}

void MapView::setMarkerColor(const QColor &color)
{
	_markerColor = color;

	for (int i = 0; i < _tracks.size(); i++)
		_tracks.at(i)->setMarkerColor(color);
	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->setMarkerColor(color);
}

void MapView::setMarkerPosition(qreal pos)
{
	for (int i = 0; i < _tracks.size(); i++)
		_tracks.at(i)->setMarkerPosition(pos);
	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->setMarkerPosition(pos);
}

void MapView::reloadMap()
{
	_scene->invalidate();
}

void MapView::setMapConfig(const Projection &in, const Projection &out,
  bool hidpi)
{
	_inputProjection = in;
	_outputProjection = out;
	_hidpi = hidpi;

	setMap(_map);
}

void MapView::setDevicePixelRatio(qreal ratio)
{
	_deviceRatio = ratio;

	setMap(_map);
}

void MapView::fitContentToSize()
{
	int zoom = _map->zoom();
	if (fitMapZoom() != zoom)
		rescale();

	centerOn(contentCenter());
}

RectC MapView::boundingRect() const
{
	RectC rect;

	if (_showTracks)
		rect |= _tr;
	if (_showRoutes)
		rect |= _rr;
	if (_showWaypoints)
		rect |= _wr;
	if (_showAreas)
		rect |= _ar;

	return rect;
}

void MapView::showPosition(bool show)
{
	_showPosition = show;

	if (_positionSource) {
		if (_showPosition) {
			_crosshair->setVisible(true);
			if (_showPositionCoordinates)
				_positionCoordinates->setVisible(true);
			if (_showMotionInfo)
				_motionInfo->setVisible(true);
			_positionSource->startUpdates();
		} else {
			_positionSource->stopUpdates();
			_crosshair->setVisible(false);
			_positionCoordinates->setVisible(false);
			_motionInfo->setVisible(false);
		}
	} else {
		_crosshair->setVisible(false);
		_positionCoordinates->setVisible(false);
		_motionInfo->setVisible(false);
	}
}

void MapView::followPosition(bool follow)
{
	_followPosition = follow;

	if (follow && _crosshair->isVisible())
		centerOn(_map->ll2xy(_crosshair->coordinates()));
}

void MapView::updatePosition(const QGeoPositionInfo &pos)
{
	QGeoCoordinate gc(pos.coordinate());
	if (!gc.isValid())
		return;

	Coordinates c(gc.longitude(), gc.latitude());
	_crosshair->setCoordinates(c);
	_crosshair->setMap(_map);
	_positionCoordinates->setCoordinates(c, gc.altitude());
	_motionInfo->setInfo(pos.attribute(QGeoPositionInfo::Direction),
	  pos.attribute(QGeoPositionInfo::GroundSpeed),
	  pos.attribute(QGeoPositionInfo::VerticalSpeed));

	if (_followPosition)
		centerOn(_map->ll2xy(c));
}

void MapView::setCrosshairColor(const QColor &color)
{
	_crosshair->setColor(color);
}

void MapView::setInfoColor(const QColor &color)
{
	_cursorCoordinates->setColor(color);
	_positionCoordinates->setColor(color);
	_motionInfo->setColor(color);
}

void MapView::drawInfoBackground(bool draw)
{
	_infoBackground = draw;

	_cursorCoordinates->drawBackground(draw);
	_positionCoordinates->drawBackground(draw);
	_motionInfo->drawBackground(draw);

	for (int i = 0; i < _tracks.size(); i++)
		_tracks.at(i)->drawMarkerBackground(draw);
	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->drawMarkerBackground(draw);
}

void MapView::setHidpi(bool hidpi)
{
	QRectF vr(mapToScene(viewport()->rect()).boundingRect()
	  .intersected(_map->bounds()));
	RectC cr(_map->xy2ll(vr.topLeft()), _map->xy2ll(vr.bottomRight()));

	_map->unload();
	_map->load(_inputProjection, _outputProjection, _deviceRatio, hidpi);
	rescale();

	QPointF nc = QRectF(_map->ll2xy(cr.topLeft()),
	  _map->ll2xy(cr.bottomRight())).center();
	centerOn(nc);

	reloadMap();
}
