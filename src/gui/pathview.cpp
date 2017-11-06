#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <QApplication>
#include <QPixmapCache>
#include <QScrollBar>
#include "opengl.h"
#include "misc.h"
#include "poi.h"
#include "data.h"
#include "map.h"
#include "trackitem.h"
#include "routeitem.h"
#include "waypointitem.h"
#include "scaleitem.h"
#include "keys.h"
#include "pathview.h"


#define MAX_DIGITAL_ZOOM 2
#define MIN_DIGITAL_ZOOM -3
#define MARGIN           10.0
#define SCALE_OFFSET     7

PathView::PathView(Map *map, POI *poi, QWidget *parent)
  : QGraphicsView(parent)
{
	Q_ASSERT(map != 0);
	Q_ASSERT(poi != 0);

	_scene = new QGraphicsScene(this);
	setScene(_scene);
	setCacheMode(QGraphicsView::CacheBackground);
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
	_opacity = 1.0;
	_backgroundColor = Qt::white;

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

	_plot = false;
	_digitalZoom = 0;

	_map->setBackgroundColor(_backgroundColor);
	_scene->setSceneRect(_map->bounds());

	centerOn(_scene->sceneRect().center());
}

void PathView::centerOn(const QPointF &pos)
{
	QGraphicsView::centerOn(pos);

	/* Fix the offset caused by QGraphicsView::centerOn() approximation */
	QPointF center = mapToScene(viewport()->rect().center());
	QPoint offset((int)(pos.x() - center.x()), (int)(pos.y() - center.y()));
	if (qAbs(offset.x()) == 1)
		horizontalScrollBar()->setValue(horizontalScrollBar()->value()
		  + offset.x());
	if (qAbs(offset.y()) == 1)
		verticalScrollBar()->setValue(verticalScrollBar()->value()
		  + offset.y());

	_res = _map->resolution(pos);
	_mapScale->setResolution(_res);
}

PathItem *PathView::addTrack(const Track &track)
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
	_scene->addItem(ti);

	addPOI(_poi->points(ti->path()));

	return ti;
}

PathItem *PathView::addRoute(const Route &route)
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
	ri->setVisible(_showRoutes);
	ri->showWaypoints(_showRouteWaypoints);
	ri->showWaypointLabels(_showWaypointLabels);
	ri->setDigitalZoom(_digitalZoom);
	_scene->addItem(ri);

	addPOI(_poi->points(ri->path()));

	return ri;
}

void PathView::addWaypoints(const QList<Waypoint> &waypoints)
{
	for (int i = 0; i < waypoints.count(); i++) {
		const Waypoint &w = waypoints.at(i);

		WaypointItem *wi = new WaypointItem(w, _map);
		_waypoints.append(wi);
		updateWaypointsBoundingRect(wi->waypoint().coordinates());
		wi->setZValue(1);
		wi->setSize(_waypointSize);
		wi->setColor(_waypointColor);
		wi->showLabel(_showWaypointLabels);
		wi->setUnits(_units);
		wi->setVisible(_showWaypoints);
		wi->setDigitalZoom(_digitalZoom);
		_scene->addItem(wi);
	}

	addPOI(_poi->points(waypoints));
}

QList<PathItem *> PathView::loadData(const Data &data)
{
	QList<PathItem *> paths;
	qreal zoom = _map->zoom();

	for (int i = 0; i < data.tracks().count(); i++)
		paths.append(addTrack(*(data.tracks().at(i))));
	for (int i = 0; i < data.routes().count(); i++)
		paths.append(addRoute(*(data.routes().at(i))));
	addWaypoints(data.waypoints());

	if (_tracks.empty() && _routes.empty() && _waypoints.empty())
		return paths;

	if (mapZoom() != zoom)
		rescale();
	else
		updatePOIVisibility();

	centerOn(contentCenter());

	return paths;
}

void PathView::updateWaypointsBoundingRect(const Coordinates &wp)
{
	if (_wr.isNull())
		_wr = RectC(wp, wp);
	else
		_wr.unite(wp);
}

qreal PathView::mapZoom() const
{
	RectC br = _tr | _rr | _wr;

	return _map->zoomFit(viewport()->size() - QSize(2*MARGIN, 2*MARGIN), br);
}

QPointF PathView::contentCenter() const
{
	RectC br = _tr | _rr | _wr;

	return _map->ll2xy(br.center());
}

void PathView::updatePOIVisibility()
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

void PathView::rescale()
{
	_scene->setSceneRect(_map->bounds());
	resetCachedContent();

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

void PathView::setPalette(const Palette &palette)
{
	_palette = palette;
	_palette.reset();

	for (int i = 0; i < _tracks.count(); i++)
		_tracks.at(i)->setColor(_palette.nextColor());
	for (int i = 0; i < _routes.count(); i++)
		_routes.at(i)->setColor(_palette.nextColor());
}

void PathView::setMap(Map *map)
{
	QPointF pos = mapToScene(viewport()->rect().center());
	Coordinates center = _map->xy2ll(pos);
	qreal resolution = _map->resolution(pos);

	_map->unload();
	disconnect(_map, SIGNAL(loaded()), this, SLOT(reloadMap()));

	_map = map;
	_map->load();
	connect(_map, SIGNAL(loaded()), this, SLOT(reloadMap()));

	resetDigitalZoom();

	_map->zoomFit(resolution, center);
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

	centerOn(_map->ll2xy(center));

	resetCachedContent();
	QPixmapCache::clear();
}

void PathView::setPOI(POI *poi)
{
	disconnect(_poi, SIGNAL(pointsChanged()), this, SLOT(updatePOI()));
	connect(poi, SIGNAL(pointsChanged()), this, SLOT(updatePOI()));

	_poi = poi;

	updatePOI();
}

void PathView::updatePOI()
{
	QHash<SearchPointer<Waypoint>, WaypointItem*>::const_iterator it;

	for (it = _pois.constBegin(); it != _pois.constEnd(); it++) {
		_scene->removeItem(it.value());
		delete it.value();
	}
	_pois.clear();

	for (int i = 0; i < _tracks.size(); i++)
		addPOI(_poi->points(_tracks.at(i)->path()));
	for (int i = 0; i < _routes.size(); i++)
		addPOI(_poi->points(_routes.at(i)->path()));
	addPOI(_poi->points(_waypoints));

	updatePOIVisibility();
}

void PathView::addPOI(const QVector<Waypoint> &waypoints)
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
		_scene->addItem(pi);

		_pois.insert(SearchPointer<Waypoint>(&(pi->waypoint())), pi);
	}
}

void PathView::setUnits(enum Units units)
{
	_units = units;

	_mapScale->setUnits(units);

	for (int i = 0; i < _tracks.count(); i++)
		_tracks[i]->setUnits(units);
	for (int i = 0; i < _routes.count(); i++)
		_routes[i]->setUnits(units);
	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->setUnits(units);

	QHash<SearchPointer<Waypoint>, WaypointItem*>::const_iterator it;
	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->setUnits(units);
}

void PathView::clearMapCache()
{
	_map->clearCache();
	resetCachedContent();
}

void PathView::resetDigitalZoom()
{
	QHash<SearchPointer<Waypoint>, WaypointItem*>::const_iterator it;

	_digitalZoom = 0;
	resetTransform();

	for (int i = 0; i < _tracks.size(); i++)
		_tracks.at(i)->setDigitalZoom(0);
	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->setDigitalZoom(0);
	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->setDigitalZoom(0);
	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->setDigitalZoom(0);

	_mapScale->setDigitalZoom(0);
}

void PathView::digitalZoom(int zoom)
{
	QHash<SearchPointer<Waypoint>, WaypointItem*>::const_iterator it;

	_digitalZoom += zoom;
	scale(pow(2, zoom), pow(2, zoom));

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

void PathView::zoom(int zoom, const QPoint &pos, const Coordinates &c)
{
	bool shift = QApplication::keyboardModifiers() & Qt::ShiftModifier;

	if (_digitalZoom) {
		if (((_digitalZoom > 0 && zoom > 0) && (!shift || _digitalZoom
		  >= MAX_DIGITAL_ZOOM)) || ((_digitalZoom < 0 && zoom < 0) && (!shift
		  || _digitalZoom <= MIN_DIGITAL_ZOOM)))
			return;

		digitalZoom(zoom);
	} else {
		qreal os, ns;
		os = _map->zoom();
		ns = (zoom > 0) ? _map->zoomIn() : _map->zoomOut();

		if (ns != os) {
			rescale();
			centerOn(_map->ll2xy(c) - (pos - viewport()->rect().center()));
		} else {
			if (shift)
				digitalZoom(zoom);
		}
	}
}

void PathView::wheelEvent(QWheelEvent *event)
{
	static int deg = 0;

	deg += event->delta() / 8;
	if (qAbs(deg) < 15)
		return;
	deg = 0;

	Coordinates c = _map->xy2ll(mapToScene(event->pos()));
	zoom((event->delta() > 0) ? 1 : -1, event->pos(), c);
}

void PathView::mouseDoubleClickEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton && event->button() != Qt::RightButton)
		return;

	Coordinates c = _map->xy2ll(mapToScene(event->pos()));
	zoom((event->button() == Qt::LeftButton) ? 1 : -1, event->pos(), c);
}

void PathView::keyPressEvent(QKeyEvent *event)
{
	int z;

	QPoint pos = viewport()->rect().center();
	Coordinates c = _map->xy2ll(mapToScene(pos));

	if (event->matches(ZOOM_IN))
		z = 1;
	else if (event->matches(ZOOM_OUT))
		z = -1;
	else if (_digitalZoom && event->key() == Qt::Key_Escape) {
		resetDigitalZoom();
		return;
	} else {
		QGraphicsView::keyPressEvent(event);
		return;
	}

	zoom(z, pos, c);
}

void PathView::plot(QPainter *painter, const QRectF &target, qreal scale,
  bool hires)
{
	QRect orig, adj;
	qreal ratio, diff, origRes, q;
	QPointF origScene, origPos;
	Coordinates origLL;


	// Enter plot mode
	setUpdatesEnabled(false);
	_plot = true;
	_map->setBlockingMode(true);

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
		origScene = mapToScene(orig.center());
		origLL = _map->xy2ll(origScene);
		origRes = _map->resolution(origScene);

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

		_mapScale->setDigitalZoom(-log2(s.x() / q));
		_mapScale->setPos(mapToScene(QPoint(adj.bottomRight() + QPoint(
		  -(SCALE_OFFSET + _mapScale->boundingRect().width()) * (s.x() / q),
		  -(SCALE_OFFSET + _mapScale->boundingRect().height()) * (s.x() / q)))));
	} else {
		_mapScale->setDigitalZoom(-log2(1.0 / q));
		_mapScale->setPos(mapToScene(QPoint(adj.bottomRight() + QPoint(
		  -(SCALE_OFFSET + _mapScale->boundingRect().width()) / q ,
		  -(SCALE_OFFSET + _mapScale->boundingRect().height()) / q))));
	}

	// Print the view
	render(painter, target, adj);

	// Revert view changes to display mode
	if (hires) {
		_map->zoomFit(origRes, origLL);
		rescale();
		centerOn(origScene);
	}
	_mapScale->setDigitalZoom(0);
	_mapScale->setPos(origPos);

	// Exit plot mode
	_map->setBlockingMode(false);
	_plot = false;
	setUpdatesEnabled(true);
}

void PathView::clear()
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

	resetDigitalZoom();
	resetCachedContent();
	QPixmapCache::clear();
}

void PathView::showTracks(bool show)
{
	_showTracks = show;

	for (int i = 0; i < _tracks.count(); i++)
		_tracks.at(i)->setVisible(show);
}

void PathView::showRoutes(bool show)
{
	_showRoutes = show;

	for (int i = 0; i < _routes.count(); i++)
		_routes.at(i)->setVisible(show);
}

void PathView::showWaypoints(bool show)
{
	_showWaypoints = show;

	for (int i = 0; i < _waypoints.count(); i++)
		_waypoints.at(i)->setVisible(show);
}

void PathView::showWaypointLabels(bool show)
{
	_showWaypointLabels = show;

	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->showLabel(show);

	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->showWaypointLabels(show);
}

void PathView::showRouteWaypoints(bool show)
{
	_showRouteWaypoints = show;

	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->showWaypoints(show);
}

void PathView::showMap(bool show)
{
	_showMap = show;
	resetCachedContent();
}

void PathView::showPOI(bool show)
{
	_showPOI = show;

	QHash<SearchPointer<Waypoint>, WaypointItem*>::const_iterator it;
	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->setVisible(show);

	updatePOIVisibility();
}

void PathView::showPOILabels(bool show)
{
	_showPOILabels = show;

	QHash<SearchPointer<Waypoint>, WaypointItem*>::const_iterator it;
	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->showLabel(show);

	updatePOIVisibility();
}

void PathView::setPOIOverlap(bool overlap)
{
	_overlapPOIs = overlap;

	updatePOIVisibility();
}

void PathView::setTrackWidth(int width)
{
	_trackWidth = width;

	for (int i = 0; i < _tracks.count(); i++)
		_tracks.at(i)->setWidth(width);
}

void PathView::setRouteWidth(int width)
{
	_routeWidth = width;

	for (int i = 0; i < _routes.count(); i++)
		_routes.at(i)->setWidth(width);
}

void PathView::setTrackStyle(Qt::PenStyle style)
{
	_trackStyle = style;

	for (int i = 0; i < _tracks.count(); i++)
		_tracks.at(i)->setStyle(style);
}

void PathView::setRouteStyle(Qt::PenStyle style)
{
	_routeStyle = style;

	for (int i = 0; i < _routes.count(); i++)
		_routes.at(i)->setStyle(style);
}

void PathView::setWaypointSize(int size)
{
	_waypointSize = size;

	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->setSize(size);
}

void PathView::setWaypointColor(const QColor &color)
{
	_waypointColor = color;

	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->setColor(color);
}

void PathView::setPOISize(int size)
{
	QHash<SearchPointer<Waypoint>, WaypointItem*>::const_iterator it;

	_poiSize = size;

	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->setSize(size);
}

void PathView::setPOIColor(const QColor &color)
{
	QHash<SearchPointer<Waypoint>, WaypointItem*>::const_iterator it;

	_poiColor = color;

	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->setColor(color);
}

void PathView::setMapOpacity(int opacity)
{
	_opacity = opacity / 100.0;
	resetCachedContent();
}

void PathView::setBackgroundColor(const QColor &color)
{
	_backgroundColor = color;
	_map->setBackgroundColor(color);
	resetCachedContent();
}

void PathView::drawBackground(QPainter *painter, const QRectF &rect)
{
	if (_showMap) {
		if (_opacity < 1.0) {
			painter->fillRect(rect, _backgroundColor);
			painter->setOpacity(_opacity);
		}
		_map->draw(painter, rect);
	} else
		painter->fillRect(rect, _backgroundColor);
}

void PathView::resizeEvent(QResizeEvent *event)
{
	qreal zoom = _map->zoom();
	if (mapZoom() != zoom)
		rescale();

	centerOn(contentCenter());

	QGraphicsView::resizeEvent(event);
}

void PathView::paintEvent(QPaintEvent *event)
{
	QPointF scenePos = mapToScene(rect().bottomRight() + QPoint(
	  -(SCALE_OFFSET + _mapScale->boundingRect().width()),
	  -(SCALE_OFFSET + _mapScale->boundingRect().height())));
	if (_mapScale->pos() != scenePos && !_plot)
		_mapScale->setPos(scenePos);

	QGraphicsView::paintEvent(event);
}

void PathView::scrollContentsBy(int dx, int dy)
{
	QGraphicsView::scrollContentsBy(dx, dy);

	QPointF center = mapToScene(viewport()->rect().center());
	qreal res = _map->resolution(center);

	if (qMax(res, _res) / qMin(res, _res) > 1.1) {
		_mapScale->setResolution(res);
		_res = res;
	}
}

void PathView::useOpenGL(bool use)
{
	if (use)
		setViewport(new OPENGL_WIDGET);
	else
		setViewport(new QWidget);
}

void PathView::useAntiAliasing(bool use)
{
	setRenderHint(QPainter::Antialiasing, use);
}

void PathView::reloadMap()
{
	resetCachedContent();
}
