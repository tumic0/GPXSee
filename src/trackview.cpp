#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QWheelEvent>
#include "poi.h"
#include "gpx.h"
#include "map.h"
#include "waypointitem.h"
#include "markeritem.h"
#include "scaleitem.h"
#include "ll.h"
#include "trackview.h"


#define MARGIN          10.0
#define TRACK_WIDTH     3
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
}

TrackView::~TrackView()
{
	if (_mapScale->scene() != _scene)
		delete _mapScale;
}

void TrackView::addTrack(const QVector<QPointF> &track)
{
	QPainterPath path;
	QGraphicsPathItem *pi;
	MarkerItem *mi;


	if (track.size() < 2) {
		_palette.color();
		return;
	}

	_tracks.append(track);

	const QPointF &p = track.at(0);
	path.moveTo(ll2mercator(QPointF(p.x(), -p.y())));
	for (int i = 1; i < track.size(); i++) {
		const QPointF &p = track.at(i);
		path.lineTo(ll2mercator(QPointF(p.x(), -p.y())));
	}

	_maxPath = qMax(path.length(), _maxPath);

	pi = new QGraphicsPathItem(path);
	_paths.append(pi);
	_zoom = qMin(_zoom, scale2zoom(trackScale()));
	_scale = mapScale(_zoom);
	QBrush brush(_palette.color(), Qt::SolidPattern);
	QPen pen(brush, TRACK_WIDTH * _scale);
	pi->setPen(pen);
	pi->setScale(1.0/_scale);
	_scene->addItem(pi);

	mi = new MarkerItem(pi);
	_markers.append(mi);
	mi->setPos(pi->path().pointAtPercent(0));
	mi->setScale(_scale);
}

void TrackView::addWaypoints(const QList<Waypoint> &waypoints)
{
	for (int i = 0; i < waypoints.count(); i++) {
		const Waypoint &w = waypoints.at(i);
		WaypointItem *wi = new WaypointItem(
		  Waypoint(ll2mercator(QPointF(w.coordinates().x(),
		    -w.coordinates().y())), w.description()));

		wi->setPos(wi->entry().coordinates() * 1.0/_scale);
		wi->setZValue(1);
		_scene->addItem(wi);

		_locations.append(wi);
		_waypoints.append(w.coordinates());
	}

	_zoom = qMin(_zoom, scale2zoom(waypointScale()));
	_scale = mapScale(_zoom);
}

void TrackView::loadGPX(const GPX &gpx)
{
	int zoom = _zoom;

	for (int i = 0; i < gpx.trackCount(); i++) {
		QVector<QPointF> track;
		gpx.track(i).track(track);
		addTrack(track);
		_maxDistance = qMax(gpx.track(i).distance(), _maxDistance);
	}

	addWaypoints(gpx.waypoints());

	if (_paths.empty() && _locations.empty())
		return;

	if ((_paths.size() > 1 && _zoom < zoom)
	  || (_locations.size() && _zoom < zoom))
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

	if (_locations.empty())
		return QRectF();

	const QPointF &p = _locations.at(0)->pos();
	bottom = p.y();
	top = p.y();
	left = p.x();
	right = p.x();

	for (int i = 1; i < _locations.size(); i++) {
		const QPointF &p = _locations.at(i)->pos();
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

	if (_locations.size() < 2)
		return mapScale(ZOOM_MAX);

	const QPointF &p = _locations.at(0)->entry().coordinates();
	bottom = p.y();
	top = p.y();
	left = p.x();
	right = p.x();

	for (int i = 1; i < _locations.size(); i++) {
		const QPointF &p = _locations.at(i)->entry().coordinates();
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

		QPen pen(_paths.at(i)->pen());
		pen.setWidthF(TRACK_WIDTH * scale);
		_paths.at(i)->setPen(pen);
	}

	for (int i = 0; i < _locations.size(); i++)
		_locations.at(i)->setPos(_locations.at(i)->entry().coordinates()
		  * 1.0/scale);

	QHash<Waypoint, WaypointItem*>::const_iterator it, jt;
	for (it = _pois.constBegin(); it != _pois.constEnd(); it++) {
		it.value()->setPos(it.value()->entry().coordinates() * 1.0/scale);
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

		WaypointItem *pi = new WaypointItem(
		  Waypoint(ll2mercator(QPointF(w.coordinates().x(),
		  -w.coordinates().y())), w.description()));

		pi->setPos(pi->entry().coordinates() * 1.0/_scale);
		pi->setZValue(1);
		_scene->addItem(pi);

		_pois.insert(w, pi);
	}
}

void TrackView::loadPOI(const POI &poi)
{
	QHash<Waypoint, WaypointItem*>::const_iterator it,jt;

	if (!_tracks.size() && !_waypoints.size())
		return;

	for (int i = 0; i < _tracks.size(); i++)
		addPOI(poi.points(_tracks.at(i)));
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
		connect(_map, SIGNAL(loaded()), this, SLOT(redraw()));
	resetCachedContent();
}

void TrackView::setUnits(enum Units units)
{
	_mapScale->setUnits(units);
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

void TrackView::wheelEvent(QWheelEvent *event)
{
	if (_paths.isEmpty() && _locations.isEmpty())
		return;

	QPointF pos = mapToScene(event->pos());
	qreal scale = _scale;

	_zoom = (event->delta() > 0) ?
		qMin(_zoom + 1, ZOOM_MAX) : qMax(_zoom - 1, ZOOM_MIN);

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

void TrackView::setTrackLineWidth(qreal width)
{
	for (int i = 0; i < _paths.size(); i++) {
		QPen pen(_paths.at(i)->pen());
		pen.setWidthF(width);
		_paths.at(i)->setPen(pen);
	}
}

void TrackView::plot(QPainter *painter, const QRectF &target)
{
	QRectF orig, adj;
	qreal ratio, diff;

	orig = viewport()->rect();

	if (target.width()/target.height() > orig.width()/orig.height()) {
		ratio = target.width()/target.height();
		diff = qAbs((orig.height() * ratio) - orig.width());
		adj = orig.adjusted(-diff/2, 0, diff/2, 0);
	} else {
		ratio = target.height()/target.width();
		diff = qAbs((orig.width() * ratio) - orig.height());
		adj = orig.adjusted(0, -diff/2, 0, diff/2);
	}

	_mapScale->setPos(mapToScene(QPointF(adj.bottomRight()
	  + QPoint(-_mapScale->boundingRect().width(),
	  -_mapScale->boundingRect().height())).toPoint()));

	render(painter, target, adj.toRect());
}

enum QPrinter::Orientation TrackView::orientation() const
{
	return (sceneRect().width() > sceneRect().height())
		? QPrinter::Landscape : QPrinter::Portrait;
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
	_locations.clear();
	_markers.clear();
	_scene->clear();
	_palette.reset();

	_tracks.clear();
	_waypoints.clear();

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
	if ((_paths.isEmpty() && _locations.isEmpty())|| !_map) {
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

	_map->loadTiles(tiles);

	for (int i = 0; i < tiles.count(); i++) {
		Tile &t = tiles[i];
		QPoint tp(tl.x() + (t.xy().rx() - tile.rx()) * TILE_SIZE,
		  tl.y() + (t.xy().ry() - tile.ry()) * TILE_SIZE);
		painter->drawPixmap(tp, t.pixmap());
	}
}

void TrackView::resizeEvent(QResizeEvent *e)
{
	if (_paths.isEmpty() && _locations.isEmpty())
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
	QPointF scenePos = mapToScene(rect().bottomLeft() + QPoint(SCALE_OFFSET,
	  -(SCALE_OFFSET + _mapScale->boundingRect().height())));
	if (_mapScale->pos() != scenePos)
		_mapScale->setPos(scenePos);

	QGraphicsView::paintEvent(e);
}
