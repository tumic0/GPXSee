#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <QSysInfo>
#include "opengl.h"
#include "misc.h"
#include "poi.h"
#include "data.h"
#include "map.h"
#include "emptymap.h"
#include "trackitem.h"
#include "routeitem.h"
#include "waypointitem.h"
#include "scaleitem.h"
#include "pathview.h"


#define MARGIN        10.0
#define SCALE_OFFSET  7

static void unite(QRectF &rect, const QPointF &p)
{
	if (p.x() < rect.left())
		rect.setLeft(p.x());
	if (p.x() > rect.right())
		rect.setRight(p.x());
	if (p.y() > rect.bottom())
		rect.setBottom(p.y());
	if (p.y() < rect.top())
		rect.setTop(p.y());
}

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

	_map = map;
	_poi = poi;
	connect(_map, SIGNAL(loaded()), this, SLOT(redraw()));
	connect(_poi, SIGNAL(pointsChanged()), this, SLOT(updatePOI()));

	_units = Metric;

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

	_plot = false;

	setSceneRect(_map->bounds());
	_res = _map->resolution(_scene->sceneRect().center());
}

PathView::~PathView()
{
	if (_mapScale->scene() != _scene)
		delete _mapScale;
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
	ti->setVisible(_showTracks);
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
	ri->setVisible(_showRoutes);
	ri->showWaypoints(_showRouteWaypoints);
	ri->showWaypointLabels(_showWaypointLabels);
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
		Coordinates c = wi->waypoint().coordinates();
		updateWaypointsBoundingRect(QPointF(c.lon(), c.lat()));
		wi->setZValue(1);
		wi->showLabel(_showWaypointLabels);
		wi->setVisible(_showWaypoints);
		_scene->addItem(wi);
	}

	addPOI(_poi->points(waypoints));
}

QList<PathItem *> PathView::loadData(const Data &data)
{
	QList<PathItem *> paths;
	qreal scale = _map->zoom();

	for (int i = 0; i < data.tracks().count(); i++)
		paths.append(addTrack(*(data.tracks().at(i))));
	for (int i = 0; i < data.routes().count(); i++)
		paths.append(addRoute(*(data.routes().at(i))));
	addWaypoints(data.waypoints());

	if (_tracks.empty() && _routes.empty() && _waypoints.empty())
		return paths;

	if (mapScale() != scale)
		rescale();
	else
		updatePOIVisibility();

	QPointF center = contentCenter();
	centerOn(center);

	_res = _map->resolution(center);
	_mapScale->setResolution(_res);
	if (_mapScale->scene() != _scene)
		_scene->addItem(_mapScale);

	return paths;
}

void PathView::updateWaypointsBoundingRect(const QPointF &wp)
{
	if (_wr.isNull()) {
		if (_wp.isNull())
			_wp = wp;
		else {
			_wr = QRectF(_wp, wp).normalized();
			_wp = QPointF();
		}
	} else
		unite(_wr, wp);
}

qreal PathView::mapScale() const
{
	QRectF br = _tr | _rr | _wr;
	if (!br.isNull() && !_wp.isNull())
		unite(br, _wp);

	return _map->zoomFit(viewport()->size() - QSize(MARGIN/2, MARGIN/2), br);
}

QPointF PathView::contentCenter() const
{
	QRectF br = _tr | _rr | _wr;
	if (!br.isNull() && !_wp.isNull())
		unite(br, _wp);

	if (br.isNull())
		return _map->ll2xy(_wp);
	else
		return _map->ll2xy(br.center());
}

void PathView::updatePOIVisibility()
{
	QHash<Waypoint, WaypointItem*>::const_iterator it, jt;

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
	setSceneRect(_map->bounds());

	for (int i = 0; i < _tracks.size(); i++)
		_tracks.at(i)->setMap(_map);
	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->setMap(_map);
	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->setMap(_map);

	QHash<Waypoint, WaypointItem*>::const_iterator it;
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
	_map->unload();
	disconnect(_map, SIGNAL(loaded()), this, SLOT(redraw()));

	_map = map;
	_map->load();
	connect(_map, SIGNAL(loaded()), this, SLOT(redraw()));

	mapScale();
	setSceneRect(_map->bounds());

	for (int i = 0; i < _tracks.size(); i++)
		_tracks.at(i)->setMap(map);
	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->setMap(map);
	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->setMap(map);

	QPointF center = contentCenter();
	centerOn(center);

	_res = _map->resolution(center);
	_mapScale->setResolution(_res);

	resetCachedContent();
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
	QHash<Waypoint, WaypointItem*>::const_iterator it;

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

		if (_pois.contains(w))
			continue;

		WaypointItem *pi = new WaypointItem(w, _map);
		pi->setZValue(1);
		pi->showLabel(_showPOILabels);
		pi->setVisible(_showPOI);
		_scene->addItem(pi);

		_pois.insert(w, pi);
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

	QHash<Waypoint, WaypointItem*>::const_iterator it;
	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->setUnits(units);
}

void PathView::redraw()
{
	resetCachedContent();
}

void PathView::zoom(const QPoint &pos, const Coordinates &c)
{
	QPoint offset = pos - viewport()->rect().center();

	rescale();

	QPointF center = _map->ll2xy(c) - offset;
	centerOn(center);

	_res = _map->resolution(center);
	_mapScale->setResolution(_res);

	resetCachedContent();
}

void PathView::wheelEvent(QWheelEvent *event)
{
	qreal os, ns;

	os = _map->zoom();
	Coordinates c = _map->xy2ll(mapToScene(event->pos()));

	ns = (event->delta() > 0) ? _map->zoomIn() : _map->zoomOut();
	if (ns != os)
		zoom(event->pos(), c);
}

void PathView::mouseDoubleClickEvent(QMouseEvent *event)
{
	qreal os, ns;

	if (event->button() != Qt::LeftButton && event->button() != Qt::RightButton)
		return;

	os = _map->zoom();
	Coordinates c = _map->xy2ll(mapToScene(event->pos()));

	ns = (event->button() == Qt::LeftButton) ? _map->zoomIn() : _map->zoomOut();
	if (ns != os)
		zoom(event->pos(), c);
}

void PathView::keyPressEvent(QKeyEvent *event)
{
	qreal os, ns;

	os = _map->zoom();
	QPoint pos = QRect(QPoint(), viewport()->size()).center();
	Coordinates c = _map->xy2ll(mapToScene(pos));

	if (event->matches(QKeySequence::ZoomIn))
		ns = _map->zoomIn();
	else if (event->matches(QKeySequence::ZoomOut))
		ns = _map->zoomOut();
	else {
		QWidget::keyPressEvent(event);
		return;
	}

	if (ns != os)
		zoom(pos, c);
}

void PathView::plot(QPainter *painter, const QRectF &target)
{
	QRect orig, adj;
	qreal ratio, diff;


	orig = viewport()->rect();

	if (orig.height() * (target.width() / target.height()) - orig.width() < 0) {
		ratio = target.height()/target.width();
		diff = (orig.width() * ratio) - orig.height();
		adj = orig.adjusted(0, -diff/2, 0, diff/2);
	} else {
		ratio = target.width() / target.height();
		diff = (orig.height() * ratio) - orig.width();
		adj = orig.adjusted(-diff/2, 0, diff/2, 0);
	}

	setUpdatesEnabled(false);
	_plot = true;
	_map->setBlockingMode(true);

	QPointF pos = _mapScale->pos();
	_mapScale->setPos(mapToScene(QPoint(adj.bottomRight() + QPoint(
	  -(SCALE_OFFSET + _mapScale->boundingRect().width()),
	  -(SCALE_OFFSET + _mapScale->boundingRect().height())))));

	render(painter, target, adj);

	_mapScale->setPos(pos);

	_map->setBlockingMode(false);
	_plot = false;
	setUpdatesEnabled(true);
}

void PathView::clear()
{
	if (_mapScale->scene() == _scene)
		_scene->removeItem(_mapScale);

	_pois.clear();
	_tracks.clear();
	_routes.clear();
	_waypoints.clear();
	_scene->clear();
	_palette.reset();

	_tr = QRectF(); _rr = QRectF(); _wr = QRectF();
	_wp = QPointF();

	setSceneRect(_map->bounds());
	_res = _map->resolution(_scene->sceneRect().center());
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

	QHash<Waypoint, WaypointItem*>::const_iterator it;
	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->setVisible(show);

	updatePOIVisibility();
}

void PathView::showPOILabels(bool show)
{
	_showPOILabels = show;

	QHash<Waypoint, WaypointItem*>::const_iterator it;
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

void PathView::drawBackground(QPainter *painter, const QRectF &rect)
{
	if (_showMap)
		_map->draw(painter, rect);
	else
		painter->fillRect(rect, Qt::white);
}

void PathView::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED(event);

	qreal scale = _map->zoom();
	if (mapScale() != scale)
		rescale();

	QPointF center = contentCenter();
	centerOn(center);

	_res = _map->resolution(center);
	_mapScale->setResolution(_res);

	resetCachedContent();
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
