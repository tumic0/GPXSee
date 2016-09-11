#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QWheelEvent>
#include "ll.h"
#include "poi.h"
#include "gpx.h"
#include "map.h"
#include "trackitem.h"
#include "routeitem.h"
#include "waypointitem.h"
#include "scaleitem.h"
#include "trackview.h"


#define MARGIN          10.0
#define SCALE_OFFSET    7

TrackView::TrackView(QWidget *parent)
	: QGraphicsView(parent)
{
	_scene = new QGraphicsScene(this);
	setScene(_scene);
	setCacheMode(QGraphicsView::CacheBackground);
	setDragMode(QGraphicsView::ScrollHandDrag);
	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setRenderHints(QPainter::Antialiasing);

	_mapScale = new ScaleItem();
	_mapScale->setZValue(2.0);

	_zoom = ZOOM_MAX;
	_scale = mapScale(_zoom);
	_map = 0;

	_units = Metric;

	_showTracks = true;
	_showRoutes = true;
	_showWaypoints = true;
	_showWaypointLabels = true;
	_showPOILabels = true;
	_overlapPOIs = true;
	_showRouteWaypoints = true;

	_plot = false;
	_markerPos = 0;
}

TrackView::~TrackView()
{
	if (_mapScale->scene() != _scene)
		delete _mapScale;
}

void TrackView::addTrack(const Track &track)
{
	if (track.isNull()) {
		_palette.color();
		return;
	}

	TrackItem *ti = new TrackItem(track);
	_tracks.append(ti);
	_zoom = qMin(_zoom, scale2zoom(trackScale()));
	_scale = mapScale(_zoom);
	ti->setScale(1.0/_scale);
	ti->setColor(_palette.color());
	ti->setVisible(_showTracks);
	ti->moveMarker(_markerPos);
	_scene->addItem(ti);
}

void TrackView::addRoute(const Route &route)
{
	if (route.isNull()) {
		_palette.color();
		return;
	}

	RouteItem *ri = new RouteItem(route);
	_routes.append(ri);
	_zoom = qMin(_zoom, scale2zoom(routeScale()));
	_scale = mapScale(_zoom);
	ri->setScale(1.0/_scale);
	ri->setColor(_palette.color());
	ri->setVisible(_showRoutes);
	ri->showWaypoints(_showRouteWaypoints);
	ri->showWaypointLabels(_showWaypointLabels);
	ri->moveMarker(_markerPos);
	_scene->addItem(ri);
}

void TrackView::addWaypoints(const QList<Waypoint> &waypoints)
{
	for (int i = 0; i < waypoints.count(); i++) {
		const Waypoint &w = waypoints.at(i);

		WaypointItem *wi = new WaypointItem(w);
		wi->setScale(1.0/_scale);
		wi->setZValue(1);
		wi->showLabel(_showWaypointLabels);
		wi->setVisible(_showWaypoints);
		_scene->addItem(wi);

		_waypoints.append(wi);
	}

	_zoom = qMin(_zoom, scale2zoom(waypointScale()));
	_scale = mapScale(_zoom);
}

void TrackView::loadGPX(const GPX &gpx)
{
	int zoom = _zoom;

	for (int i = 0; i < gpx.tracks().count(); i++)
		addTrack(*(gpx.tracks().at(i)));
	for (int i = 0; i < gpx.routes().count(); i++)
		addRoute(*(gpx.routes().at(i)));
	addWaypoints(gpx.waypoints());

	if (_tracks.empty() && _routes.empty() && _waypoints.empty())
		return;

	if ((_tracks.size() > 1 && _zoom < zoom)
	  || (_routes.size() > 1 && _zoom < zoom)
	  || (_waypoints.size() && _zoom < zoom))
		rescale(_scale);

	QRectF br = trackBoundingRect() | routeBoundingRect()
	  | waypointBoundingRect();
	QRectF ba = br.adjusted(-TILE_SIZE, -TILE_SIZE, TILE_SIZE, TILE_SIZE);
	_scene->setSceneRect(ba);
	centerOn(ba.center());

	_mapScale->setZoom(_zoom, -(br.center().ry() * _scale));
	if (_mapScale->scene() != _scene)
		_scene->addItem(_mapScale);
}

QRectF TrackView::trackBoundingRect() const
{
	if (_tracks.empty())
		return QRectF();

	QRectF br = _tracks.at(0)->sceneBoundingRect();
	for (int i = 1; i < _tracks.size(); i++)
		br |= _tracks.at(i)->sceneBoundingRect();

	return br;
}

QRectF TrackView::routeBoundingRect() const
{
	if (_routes.empty())
		return QRectF();

	QRectF br = _routes.at(0)->sceneBoundingRect();
	for (int i = 1; i < _routes.size(); i++)
		br |= _routes.at(i)->sceneBoundingRect();

	return br;
}

QRectF TrackView::waypointBoundingRect() const
{
	qreal bottom, top, left, right;

	if (_waypoints.empty())
		return QRectF();

	const QPointF &p = _waypoints.at(0)->pos();
	bottom = p.y();
	top = p.y();
	left = p.x();
	right = p.x();

	for (int i = 1; i < _waypoints.size(); i++) {
		const QPointF &p = _waypoints.at(i)->pos();
		bottom = qMax(bottom, p.y());
		top = qMin(top, p.y());
		right = qMax(right, p.x());
		left = qMin(left, p.x());
	}

	return QRectF(QPointF(left, top), QPointF(right, bottom));
}

qreal TrackView::trackScale() const
{
	if (_tracks.empty())
		return mapScale(ZOOM_MAX);

	QRectF br = _tracks.at(0)->path().boundingRect();

	for (int i = 1; i < _tracks.size(); i++)
		br |= _tracks.at(i)->path().boundingRect();

	QPointF sc(br.width() / (viewport()->width() - MARGIN/2),
	  br.height() / (viewport()->height() - MARGIN/2));

	return qMax(sc.x(), sc.y());
}

qreal TrackView::routeScale() const
{
	if (_routes.empty())
		return mapScale(ZOOM_MAX);

	QRectF br = _routes.at(0)->path().boundingRect();

	for (int i = 1; i < _routes.size(); i++)
		br |= _routes.at(i)->path().boundingRect();

	QPointF sc(br.width() / (viewport()->width() - MARGIN/2),
	  br.height() / (viewport()->height() - MARGIN/2));

	return qMax(sc.x(), sc.y());
}

qreal TrackView::waypointScale() const
{
	qreal bottom, top, left, right;

	if (_waypoints.size() < 2)
		return mapScale(ZOOM_MAX);

	const QPointF &p = _waypoints.at(0)->coordinates();
	bottom = p.y();
	top = p.y();
	left = p.x();
	right = p.x();

	for (int i = 1; i < _waypoints.size(); i++) {
		const QPointF &p = _waypoints.at(i)->coordinates();
		bottom = qMax(bottom, p.y());
		top = qMin(top, p.y());
		right = qMax(right, p.x());
		left = qMin(left, p.x());
	}

	QRectF br(QPointF(left, top), QPointF(right, bottom));
	QPointF sc(br.width() / (viewport()->width() - MARGIN/2),
	  br.height() / (viewport()->height() - MARGIN/2));

	return qMax(sc.x(), sc.y());
}

qreal TrackView::mapScale(int zoom) const
{
	return ((360.0/(qreal)(1<<zoom))/(qreal)TILE_SIZE);
}

void TrackView::checkPOIOverlap()
{
	QHash<Waypoint, WaypointItem*>::const_iterator it, jt;

	for (it = _pois.constBegin(); it != _pois.constEnd(); it++) {
		for (jt = _pois.constBegin(); jt != _pois.constEnd(); jt++) {
			if (it != jt && it.value()->isVisible() && jt.value()->isVisible()
			  && it.value()->collidesWithItem(jt.value()))
				jt.value()->hide();
		}
	}
}

void TrackView::rescale(qreal scale)
{
	for (int i = 0; i < _tracks.size(); i++)
		_tracks.at(i)->setScale(1.0/scale);

	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->setScale(1.0/scale);

	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->setScale(1.0/scale);

	QHash<Waypoint, WaypointItem*>::const_iterator it;
	for (it = _pois.constBegin(); it != _pois.constEnd(); it++) {
		it.value()->setScale(1.0/scale);
		it.value()->show();
	}

	if (!_overlapPOIs)
		checkPOIOverlap();

	_scale = scale;
}

void TrackView::addPOI(const QVector<Waypoint> &waypoints)
{
	for (int i = 0; i < waypoints.size(); i++) {
		const Waypoint &w = waypoints.at(i);

		if (_pois.contains(w))
			continue;

		WaypointItem *pi = new WaypointItem(w);
		pi->setScale(1.0/_scale);
		pi->setZValue(1);
		pi->showLabel(_showPOILabels);
		_scene->addItem(pi);

		_pois.insert(w, pi);
	}
}

void TrackView::loadPOI(const POI &poi)
{
	if (!_tracks.size() && !_routes.size() && !_waypoints.size())
		return;

	for (int i = 0; i < _tracks.size(); i++)
		addPOI(poi.points(_tracks.at(i)->path()));
	for (int i = 0; i < _routes.size(); i++)
		addPOI(poi.points(_routes.at(i)->path()));
	addPOI(poi.points(_waypoints));

	if (!_overlapPOIs)
		checkPOIOverlap();
}

void TrackView::setMap(Map *map)
{
	_map = map;
	if (_map)
		connect(_map, SIGNAL(loaded()), this, SLOT(redraw()),
		  Qt::UniqueConnection);
	resetCachedContent();
}

void TrackView::setUnits(enum Units units)
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

void TrackView::redraw()
{
	resetCachedContent();
}

void TrackView::rescale()
{
	_zoom = qMin(qMin(scale2zoom(trackScale()), scale2zoom(routeScale())),
	  scale2zoom(waypointScale()));

	rescale(mapScale(_zoom));
	_mapScale->setZoom(_zoom);
}

void TrackView::zoom(int z, const QPointF &pos)
{
	if (_tracks.isEmpty() && _routes.isEmpty() && _waypoints.isEmpty())
		return;

	qreal scale = _scale;
	_zoom = z;

	rescale(mapScale(_zoom));
	QRectF br = trackBoundingRect() | routeBoundingRect()
	  | waypointBoundingRect();
	QRectF ba = br.adjusted(-TILE_SIZE, -TILE_SIZE, TILE_SIZE, TILE_SIZE);
	_scene->setSceneRect(ba);

	if (br.width() < viewport()->size().width()
	  && br.height() < viewport()->size().height())
		centerOn(br.center());
	else
		centerOn(pos * scale/_scale);

	_mapScale->setZoom(_zoom);

	resetCachedContent();
}

void TrackView::wheelEvent(QWheelEvent *event)
{
	if (_tracks.isEmpty() && _routes.isEmpty() && _waypoints.isEmpty())
		return;

	QPointF pos = mapToScene(event->pos());
	int z = (event->delta() > 0) ?
		qMin(_zoom + 1, ZOOM_MAX) : qMax(_zoom - 1, ZOOM_MIN);

	zoom(z, pos);
}

void TrackView::keyPressEvent(QKeyEvent *event)
{
	int z = -1;

	if (event->matches(QKeySequence::ZoomIn))
		z = qMin(_zoom + 1, ZOOM_MAX);
	if (event->matches(QKeySequence::ZoomOut))
		z = qMax(_zoom - 1, ZOOM_MIN);

	if (z >= 0)
		zoom(z, mapToScene(QRect(QPoint(), size()).center()));
	else
		QWidget::keyPressEvent(event);
}

void TrackView::plot(QPainter *painter, const QRectF &target)
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

	QPointF pos = _mapScale->pos();
	_mapScale->setPos(mapToScene(QPoint(adj.bottomRight() + QPoint(
	  -(SCALE_OFFSET + _mapScale->boundingRect().width()),
	  -(SCALE_OFFSET + _mapScale->boundingRect().height())))));

	render(painter, target, adj);

	_mapScale->setPos(pos);

	_plot = false;
	setUpdatesEnabled(true);
}

void TrackView::clearPOI()
{
	QHash<Waypoint, WaypointItem*>::const_iterator it;

	for (it = _pois.constBegin(); it != _pois.constEnd(); it++) {
		_scene->removeItem(it.value());
		delete it.value();
	}

	_pois.clear();
}

void TrackView::clear()
{
	if (_mapScale->scene() == _scene)
		_scene->removeItem(_mapScale);

	_pois.clear();
	_tracks.clear();
	_routes.clear();
	_waypoints.clear();
	_scene->clear();
	_palette.reset();

	_zoom = ZOOM_MAX;
	_scale = mapScale(_zoom);

	_scene->setSceneRect(QRectF());

	_markerPos = 0;
}

void TrackView::movePositionMarker(qreal val)
{
	_markerPos = val;

	for (int i = 0; i < _tracks.size(); i++)
		_tracks.at(i)->moveMarker(val);

	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->moveMarker(val);
}

void TrackView::showTracks(bool show)
{
	_showTracks = show;

	for (int i = 0; i < _tracks.count(); i++)
		_tracks.at(i)->setVisible(show);
}

void TrackView::showRoutes(bool show)
{
	_showRoutes = show;

	for (int i = 0; i < _routes.count(); i++)
		_routes.at(i)->setVisible(show);
}

void TrackView::showWaypoints(bool show)
{
	_showWaypoints = show;

	for (int i = 0; i < _waypoints.count(); i++)
		_waypoints.at(i)->setVisible(show);
}

void TrackView::showWaypointLabels(bool show)
{
	_showWaypointLabels = show;

	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->showLabel(show);

	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->showWaypointLabels(show);
}

void TrackView::showRouteWaypoints(bool show)
{
	_showRouteWaypoints = show;

	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->showWaypoints(show);
}

void TrackView::showPOILabels(bool show)
{
	_showPOILabels = show;

	QHash<Waypoint, WaypointItem*>::const_iterator it;
	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->showLabel(show);

	setPOIOverlap(_overlapPOIs);
}

void TrackView::setPOIOverlap(bool overlap)
{
	_overlapPOIs = overlap;

	if (_overlapPOIs) {
		QHash<Waypoint, WaypointItem*>::const_iterator it;
		for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
			it.value()->show();
	} else
		checkPOIOverlap();
}

void TrackView::drawBackground(QPainter *painter, const QRectF &rect)
{
	if ((_tracks.isEmpty() && _routes.isEmpty() && _waypoints.isEmpty())
	  || !_map) {
		painter->fillRect(rect, Qt::white);
		return;
	}

	QRectF rr(rect.topLeft() * _scale, rect.size());
	QPoint tile = mercator2tile(QPointF(rr.topLeft().x(), -rr.topLeft().y()),
	  _zoom);
	QPointF tm = tile2mercator(tile, _zoom);
	QPoint tl = mapToScene(mapFromScene(QPointF(tm.x() / _scale,
	  -tm.y() / _scale))).toPoint();

	QList<Tile> tiles;
	for (int i = 0; i <= rr.size().width() / TILE_SIZE + 1; i++) {
		for (int j = 0; j <= rr.size().height() / TILE_SIZE + 1; j++) {
			tiles.append(Tile(QPoint(tile.x() + i, tile.y() + j), _zoom));
		}
	}

	_map->loadTiles(tiles, _plot);

	for (int i = 0; i < tiles.count(); i++) {
		Tile &t = tiles[i];
		QPoint tp(tl.x() + (t.xy().x() - tile.x()) * TILE_SIZE,
		  tl.y() + (t.xy().y() - tile.y()) * TILE_SIZE);
		painter->drawPixmap(tp, t.pixmap());
	}
}

void TrackView::resizeEvent(QResizeEvent *e)
{
	if (_tracks.isEmpty() && _routes.isEmpty() && _waypoints.isEmpty())
		return;

	rescale();

	QRectF br = trackBoundingRect() | routeBoundingRect()
	  | waypointBoundingRect();
	QRectF ba = br.adjusted(-TILE_SIZE, -TILE_SIZE, TILE_SIZE, TILE_SIZE);

	if (ba.width() < e->size().width()) {
		qreal diff = e->size().width() - ba.width();
		ba.adjust(-diff/2, 0, diff/2, 0);
	}
	if (ba.height() < e->size().height()) {
		qreal diff = e->size().height() - ba.height();
		ba.adjust(0, -diff/2, 0, diff/2);
	}

	_scene->setSceneRect(ba);

	centerOn(br.center());
	resetCachedContent();
}

void TrackView::paintEvent(QPaintEvent *e)
{
	QPointF scenePos = mapToScene(rect().bottomRight() + QPoint(
	  -(SCALE_OFFSET + _mapScale->boundingRect().width()),
	  -(SCALE_OFFSET + _mapScale->boundingRect().height())));
	if (_mapScale->pos() != scenePos && !_plot)
		_mapScale->setPos(scenePos);

	QGraphicsView::paintEvent(e);
}
