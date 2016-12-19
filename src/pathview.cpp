#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <QSysInfo>
#include "opengl.h"
#include "rd.h"
#include "poi.h"
#include "data.h"
#include "map.h"
#include "trackitem.h"
#include "routeitem.h"
#include "waypointitem.h"
#include "scaleitem.h"
#include "pathview.h"


#define ZOOM_MAX      18
#define ZOOM_MIN      3
#define MARGIN        10.0
#define SCALE_OFFSET  7

static QPoint mercator2tile(const QPointF &m, int z)
{
	QPoint tile;

	tile.setX((int)(floor((m.x() + 180.0) / 360.0 * pow(2.0, z))));
	tile.setY((int)(floor((1.0 - (m.y() / 180.0)) / 2.0 * pow(2.0, z))));

	return tile;
}

static QPointF tile2mercator(const QPoint &tile, int z)
{
	Coordinates m;

	m.setLon(tile.x() / pow(2.0, z) * 360.0 - 180);
	qreal n = M_PI - 2.0 * M_PI * tile.y() / pow(2.0, z);
	m.setLat(rad2deg(atan(0.5 * (exp(n) - exp(-n)))));

	return m.toMercator();
}

static int scale2zoom(qreal scale)
{
	int zoom = (int)log2(360.0/(scale * (qreal)Tile::size()));

	if (zoom < ZOOM_MIN)
		return ZOOM_MIN;
	if (zoom > ZOOM_MAX)
		return ZOOM_MAX;

	return zoom;
}

qreal mapScale(int zoom)
{
	return ((360.0/(qreal)(1<<zoom))/(qreal)Tile::size());
}

static QRectF qrectf(const QPointF &p1, const QPointF &p2)
{
	return QRectF(QPointF(qMin(p1.x(), p2.x()), qMin(p1.y(), p2.y())),
	  QPointF(qMax(p1.x(), p2.x()), qMax(p1.y(), p2.y())));
}

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

static QRectF scaled(const QRectF &rect, qreal factor)
{
	return QRectF(QPointF(rect.left() * factor, rect.top() * factor),
	  QSizeF(rect.width() * factor, rect.height() * factor));
}


PathView::PathView(QWidget *parent)
	: QGraphicsView(parent)
{
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

	_zoom = ZOOM_MAX;
	_map = 0;
	_poi = 0;

	_units = Metric;

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

	TrackItem *ti = new TrackItem(track);
	_tracks.append(ti);
	_tr |= ti->path().boundingRect();
	_zoom = scale2zoom(contentsScale());
	ti->setScale(1.0/mapScale(_zoom));
	ti->setColor(_palette.nextColor());
	ti->setWidth(_trackWidth);
	ti->setStyle(_trackStyle);
	ti->setVisible(_showTracks);
	_scene->addItem(ti);

	if (_poi)
		addPOI(_poi->points(ti));

	return ti;
}

PathItem *PathView::addRoute(const Route &route)
{
	if (route.isNull()) {
		_palette.nextColor();
		return 0;
	}

	RouteItem *ri = new RouteItem(route);
	_routes.append(ri);
	_rr |= ri->path().boundingRect();
	_zoom = scale2zoom(contentsScale());
	ri->setScale(1.0/mapScale(_zoom));
	ri->setColor(_palette.nextColor());
	ri->setWidth(_routeWidth);
	ri->setStyle(_routeStyle);
	ri->setVisible(_showRoutes);
	ri->showWaypoints(_showRouteWaypoints);
	ri->showWaypointLabels(_showWaypointLabels);
	_scene->addItem(ri);

	if (_poi)
		addPOI(_poi->points(ri));

	return ri;
}

void PathView::addWaypoints(const QList<Waypoint> &waypoints)
{
	qreal scale = mapScale(_zoom);

	for (int i = 0; i < waypoints.count(); i++) {
		const Waypoint &w = waypoints.at(i);

		WaypointItem *wi = new WaypointItem(w);
		wi->setScale(1.0/scale);
		wi->setZValue(1);
		wi->showLabel(_showWaypointLabels);
		wi->setVisible(_showWaypoints);
		_scene->addItem(wi);

		if (_wr.isNull()) {
			if (_wp.isNull())
				_wp = wi->coordinates();
			else
				_wr = qrectf(_wp, wi->coordinates());
		} else
			unite(_wr, wi->coordinates());

		_waypoints.append(wi);
	}

	if (_poi)
		addPOI(_poi->points(waypoints));

	_zoom = scale2zoom(contentsScale());
}

QList<PathItem *> PathView::loadData(const Data &data)
{
	QList<PathItem *> paths;
	int zoom = _zoom;


	for (int i = 0; i < data.tracks().count(); i++)
		paths.append(addTrack(*(data.tracks().at(i))));
	for (int i = 0; i < data.routes().count(); i++)
		paths.append(addRoute(*(data.routes().at(i))));
	addWaypoints(data.waypoints());

	if (_tracks.empty() && _routes.empty() && _waypoints.empty())
		return paths;

	if ((_tracks.size() + _routes.size() > 1 && _zoom < zoom)
	  || (_waypoints.size() && _zoom < zoom))
		rescale(_zoom);
	else
		updatePOIVisibility();

	QRectF sr = contentsSceneRect();
	_scene->setSceneRect(sr);
	centerOn(sr.center());

	_mapScale->setZoom(_zoom, -(sr.center().ry() * mapScale(_zoom)));
	if (_mapScale->scene() != _scene)
		_scene->addItem(_mapScale);

	return paths;
}

qreal PathView::contentsScale() const
{
	QRectF br = _tr | _rr | _wr;

	if (br.isNull())
		return mapScale(ZOOM_MAX);

	QPointF sc(br.width() / (viewport()->width() - MARGIN/2),
	  br.height() / (viewport()->height() - MARGIN/2));

	return qMax(sc.x(), sc.y());
}

QRectF PathView::contentsSceneRect() const
{
	qreal scale = mapScale(_zoom);
	QRectF br = scaled(_tr | _rr | _wr, 1.0/scale);

	if (br.isNull())
		return QRectF(QPointF(_wp.x() / scale - Tile::size()/2,
		  _wp.y() /scale - Tile::size()/2), QSizeF(Tile::size(), Tile::size()));
	else
		return br.adjusted(-Tile::size(), -Tile::size(), Tile::size(),
		  Tile::size());
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

void PathView::rescale(int zoom)
{
	_zoom = zoom;
	qreal scale = mapScale(zoom);

	for (int i = 0; i < _tracks.size(); i++)
		_tracks.at(i)->setScale(1.0/scale);

	for (int i = 0; i < _routes.size(); i++)
		_routes.at(i)->setScale(1.0/scale);

	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->setScale(1.0/scale);

	QHash<Waypoint, WaypointItem*>::const_iterator it;
	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->setScale(1.0/scale);

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

void PathView::setPOI(POI *poi)
{
	if (_poi)
		disconnect(_poi, SIGNAL(pointsChanged()), this, SLOT(updatePOI()));

	_poi = poi;

	if (_poi)
		connect(_poi, SIGNAL(pointsChanged()), this, SLOT(updatePOI()));

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

	if (!_poi)
		return;

	for (int i = 0; i < _tracks.size(); i++)
		addPOI(_poi->points(_tracks.at(i)));
	for (int i = 0; i < _routes.size(); i++)
		addPOI(_poi->points(_routes.at(i)));
	addPOI(_poi->points(_waypoints));

	updatePOIVisibility();
}

void PathView::addPOI(const QVector<Waypoint> &waypoints)
{
	qreal scale = mapScale(_zoom);

	for (int i = 0; i < waypoints.size(); i++) {
		const Waypoint &w = waypoints.at(i);

		if (_pois.contains(w))
			continue;

		WaypointItem *pi = new WaypointItem(w);
		pi->setScale(1.0/scale);
		pi->setZValue(1);
		pi->showLabel(_showPOILabels);
		pi->setVisible(_showPOI);
		_scene->addItem(pi);

		_pois.insert(w, pi);
	}
}

void PathView::setMap(Map *map)
{
	if (_map)
		disconnect(_map, SIGNAL(loaded()), this, SLOT(redraw()));

	_map = map;

	if (_map)
		connect(_map, SIGNAL(loaded()), this, SLOT(redraw()));

	resetCachedContent();
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

void PathView::rescale()
{
	int zoom = scale2zoom(contentsScale());

	if (zoom != _zoom) {
		rescale(zoom);
		_mapScale->setZoom(zoom);
	}
}

void PathView::zoom(int z, const QPoint &pos)
{
	if (_tracks.isEmpty() && _routes.isEmpty() && _waypoints.isEmpty())
		return;

	QPoint offset = pos - viewport()->rect().center();
	QPointF spos = mapToScene(pos);

	qreal os = mapScale(_zoom);
	_zoom = z;

	rescale(_zoom);

	QRectF sr = contentsSceneRect();
	_scene->setSceneRect(sr);

	if (sr.width() < viewport()->size().width()
	  && sr.height() < viewport()->size().height())
		centerOn(sr.center());
	else
		centerOn((spos * (os/mapScale(_zoom))) - offset);

	_mapScale->setZoom(_zoom);

	resetCachedContent();
}

void PathView::wheelEvent(QWheelEvent *event)
{
	int z = (event->delta() > 0) ?
		qMin(_zoom + 1, ZOOM_MAX) : qMax(_zoom - 1, ZOOM_MIN);

	zoom(z, event->pos());
}

void PathView::mouseDoubleClickEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton && event->button() != Qt::RightButton)
		return;

	int z = (event->button() == Qt::LeftButton) ?
		qMin(_zoom + 1, ZOOM_MAX) : qMax(_zoom - 1, ZOOM_MIN);

	zoom(z, event->pos());
}

void PathView::keyPressEvent(QKeyEvent *event)
{
	int z = -1;

	if (event->matches(QKeySequence::ZoomIn))
		z = qMin(_zoom + 1, ZOOM_MAX);
	if (event->matches(QKeySequence::ZoomOut))
		z = qMax(_zoom - 1, ZOOM_MIN);

	if (z >= 0)
		zoom(z, QRect(QPoint(), size()).center());
	else
		QWidget::keyPressEvent(event);
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

	QPointF pos = _mapScale->pos();
	_mapScale->setPos(mapToScene(QPoint(adj.bottomRight() + QPoint(
	  -(SCALE_OFFSET + _mapScale->boundingRect().width()),
	  -(SCALE_OFFSET + _mapScale->boundingRect().height())))));

	render(painter, target, adj);

	_mapScale->setPos(pos);

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

	_zoom = ZOOM_MAX;
	_tr = QRectF(); _rr = QRectF(); _wr = QRectF();
	_wp = QPointF();

	_scene->setSceneRect(QRectF());
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
	if ((_tracks.isEmpty() && _routes.isEmpty() && _waypoints.isEmpty())
	  || !_map) {
		painter->fillRect(rect, Qt::white);
		return;
	}

	qreal scale = mapScale(_zoom);
	QRectF rr(rect.topLeft() * scale, rect.size());
	QPoint tile = mercator2tile(QPointF(rr.topLeft().x(), -rr.topLeft().y()),
	  _zoom);
	QPointF tm = tile2mercator(tile, _zoom);
	QPoint tl = mapToScene(mapFromScene(QPointF(tm.x() / scale,
	  -tm.y() / scale))).toPoint();

	QList<Tile> tiles;
	for (int i = 0; i <= rr.size().width() / Tile::size() + 1; i++) {
		for (int j = 0; j <= rr.size().height() / Tile::size() + 1; j++) {
			tiles.append(Tile(QPoint(tile.x() + i, tile.y() + j), _zoom));
		}
	}

	_map->loadTiles(tiles, _plot);

	for (int i = 0; i < tiles.count(); i++) {
		Tile &t = tiles[i];
		QPoint tp(tl.x() + (t.xy().x() - tile.x()) * Tile::size(),
		  tl.y() + (t.xy().y() - tile.y()) * Tile::size());
		painter->drawPixmap(tp, t.pixmap());
	}
}

void PathView::resizeEvent(QResizeEvent *event)
{
	if (_tracks.isEmpty() && _routes.isEmpty() && _waypoints.isEmpty())
		return;

	rescale();

	QRectF sr = contentsSceneRect();

	if (sr.width() < event->size().width()) {
		qreal diff = event->size().width() - sr.width();
		sr.adjust(-diff/2, 0, diff/2, 0);
	}
	if (sr.height() < event->size().height()) {
		qreal diff = event->size().height() - sr.height();
		sr.adjust(0, -diff/2, 0, diff/2);
	}

	_scene->setSceneRect(sr);
	centerOn(sr.center());

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

void PathView::useOpenGL(bool use)
{
	if (use) {
#ifdef Q_OS_WIN32
		if (QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA)
#endif // Q_OS_WIN32
	setViewport(new OPENGL_WIDGET);
	} else
		setViewport(new QWidget);
}
