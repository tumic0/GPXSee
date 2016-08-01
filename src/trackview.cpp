#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QWheelEvent>
#include "ll.h"
#include "poi.h"
#include "gpx.h"
#include "map.h"
#include "trackitem.h"
#include "waypointitem.h"
#include "markeritem.h"
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
	_maxPath = 0;
	_maxDistance = 0;

	_plot = false;
	_units = Metric;
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

	TrackItem *pi = new TrackItem(track);
	_paths.append(pi);
	_zoom = qMin(_zoom, scale2zoom(trackScale()));
	_scale = mapScale(_zoom);
	pi->setScale(1.0/_scale);
	pi->setColor(_palette.color());
	_scene->addItem(pi);

	MarkerItem *mi = new MarkerItem(pi);
	_markers.append(mi);
	mi->setPos(pi->path().pointAtPercent(0));
	mi->setScale(_scale);

	_maxPath = qMax(pi->path().length(), _maxPath);
	_maxDistance = qMax(track.distance(), _maxDistance);
}

void TrackView::addWaypoints(const QList<Waypoint> &waypoints)
{
	for (int i = 0; i < waypoints.count(); i++) {
		const Waypoint &w = waypoints.at(i);

		WaypointItem *wi = new WaypointItem(w);
		wi->setScale(1.0/_scale);
		wi->setZValue(1);
		_scene->addItem(wi);

		_waypoints.append(wi);
	}

	_zoom = qMin(_zoom, scale2zoom(waypointScale()));
	_scale = mapScale(_zoom);
}

void TrackView::loadGPX(const GPX &gpx)
{
	int zoom = _zoom;

	for (int i = 0; i < gpx.trackCount(); i++)
		addTrack(gpx.track(i));
	addWaypoints(gpx.waypoints());

	if (_paths.empty() && _waypoints.empty())
		return;

	if ((_paths.size() > 1 && _zoom < zoom)
	  || (_waypoints.size() && _zoom < zoom))
		rescale(_scale);

	QRectF br = trackBoundingRect() | waypointBoundingRect();
	QRectF ba = br.adjusted(-TILE_SIZE, -TILE_SIZE, TILE_SIZE, TILE_SIZE);
	_scene->setSceneRect(ba);
	centerOn(ba.center());

	_mapScale->setLatitude(-(br.center().ry() * _scale));
	_mapScale->setZoom(_zoom);
	if (_mapScale->scene() != _scene)
		_scene->addItem(_mapScale);
}

QRectF TrackView::trackBoundingRect() const
{
	if (_paths.empty())
		return QRectF();

	QRectF br = _paths.at(0)->sceneBoundingRect();
	for (int i = 1; i < _paths.size(); i++)
		br |= _paths.at(i)->sceneBoundingRect();

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
	if (_paths.empty())
		return mapScale(ZOOM_MAX);

	QRectF br = _paths.at(0)->path().boundingRect();

	for (int i = 1; i < _paths.size(); i++)
		br |= _paths.at(i)->path().boundingRect();

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

void TrackView::rescale(qreal scale)
{
	for (int i = 0; i < _paths.size(); i++) {
		_markers.at(i)->setScale(scale);
		_paths.at(i)->setScale(1.0/scale);
	}

	for (int i = 0; i < _waypoints.size(); i++)
		_waypoints.at(i)->setScale(1.0/scale);

	QHash<Waypoint, WaypointItem*>::const_iterator it, jt;
	for (it = _pois.constBegin(); it != _pois.constEnd(); it++) {
		it.value()->setScale(1.0/scale);
		it.value()->show();
	}

	for (it = _pois.constBegin(); it != _pois.constEnd(); it++) {
		for (jt = _pois.constBegin(); jt != _pois.constEnd(); jt++) {
			if (it != jt && it.value()->isVisible() && jt.value()->isVisible()
			  && it.value()->collidesWithItem(jt.value()))
				jt.value()->hide();
		}
	}

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
		_scene->addItem(pi);

		_pois.insert(w, pi);
	}
}

void TrackView::loadPOI(const POI &poi)
{
	QHash<Waypoint, WaypointItem*>::const_iterator it,jt;

	if (!_paths.size() && !_waypoints.size())
		return;

	for (int i = 0; i < _paths.size(); i++)
		addPOI(poi.points(_paths.at(i)->path()));
	addPOI(poi.points(_waypoints));

	for (it = _pois.constBegin(); it != _pois.constEnd(); it++) {
		for (jt = _pois.constBegin(); jt != _pois.constEnd(); jt++) {
			if (it != jt && it.value()->isVisible() && jt.value()->isVisible()
			  && it.value()->collidesWithItem(jt.value()))
				jt.value()->hide();
		}
	}
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

	for (int i = 0; i < _paths.count(); i++)
		_paths[i]->setUnits(units);

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
	_zoom = qMin(scale2zoom(trackScale()), scale2zoom(waypointScale()));
	rescale(mapScale(_zoom));
	_mapScale->setZoom(_zoom);
}

void TrackView::zoom(int z, const QPointF &pos)
{
	if (_paths.isEmpty() && _waypoints.isEmpty())
		return;

	qreal scale = _scale;
	_zoom = z;

	rescale(mapScale(_zoom));
	QRectF br = trackBoundingRect() | waypointBoundingRect();
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
	if (_paths.isEmpty() && _waypoints.isEmpty())
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
	_paths.clear();
	_waypoints.clear();
	_markers.clear();
	_scene->clear();
	_palette.reset();

	_maxPath = 0;
	_maxDistance = 0;
	_zoom = ZOOM_MAX;
	_scale = mapScale(_zoom);

	_scene->setSceneRect(QRectF());
}

void TrackView::movePositionMarker(qreal val)
{
	qreal mp = val / _maxDistance;

	for (int i = 0; i < _paths.size(); i++) {
		qreal f = _maxPath / _paths.at(i)->path().length();
		if (mp * f < 0 || mp * f > 1.0)
			_markers.at(i)->setVisible(false);
		else {
			QPointF pos = _paths.at(i)->path().pointAtPercent(mp * f);
			_markers.at(i)->setPos(pos);
			_markers.at(i)->setVisible(true);
		}
	}
}

void TrackView::drawBackground(QPainter *painter, const QRectF &rect)
{
	if ((_paths.isEmpty() && _waypoints.isEmpty()) || !_map) {
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
	if (_paths.isEmpty() && _waypoints.isEmpty())
		return;

	rescale();

	QRectF br = trackBoundingRect() | waypointBoundingRect();
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
