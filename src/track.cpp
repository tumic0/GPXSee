#include <cmath>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QWheelEvent>
#include <QGraphicsEllipseItem>
#include "poiitem.h"
#include "markeritem.h"
#include "ll.h"
#include "track.h"

#include <QDebug>


#define MARGIN 10.0
#define TRACK_WIDTH 3
#define ZOOM_MAX 19
#define ZOOM_MIN 0


Track::Track(QWidget *parent)
	: QGraphicsView(parent)
{
	_scene = new QGraphicsScene(this);
	setScene(_scene);
	setResizeAnchor(QGraphicsView::AnchorViewCenter);
	setCacheMode(QGraphicsView::CacheBackground);

	_zoom = -1;
	_scale = 1.0;
	_map = 0;
	_maxLen = 0;
}

Track::~Track()
{
	delete _scene;
}

void Track::loadGPX(const GPX &gpx)
{
	QVector<QPointF> track;
	QPainterPath path;
	QGraphicsPathItem *pi;
	MarkerItem *mi;
	QColor color = _colorShop.color();
	qreal prevScale = _scale;


	gpx.track(track);

	if (track.size() < 2)
		return;

	_tracks.append(track);

	path.moveTo(track.at(0).x(), -track.at(0).y());
	for (int i = 1; i < track.size(); i++)
		path.lineTo(track.at(i).x(), -track.at(i).y());

	_maxLen = qMax(path.length(), _maxLen);


	pi = new QGraphicsPathItem(path);
	_trackPaths.append(pi);
	_zoom = scale2zoom(trackScale());
	_scale = mapScale();
	QBrush brush(color, Qt::SolidPattern);
	QPen pen(brush, TRACK_WIDTH * _scale);
	pi->setPen(pen);
	pi->setScale(1.0/_scale);
	_scene->addItem(pi);

	mi = new MarkerItem(pi);
	_markers.append(mi);
	mi->setPos(pi->path().pointAtPercent(0));
	mi->setScale(_scale);

	if (_trackPaths.size() > 1 && prevScale != _scale)
		rescale(_scale);
	_scene->setSceneRect(_scene->itemsBoundingRect());
}

qreal Track::trackScale() const
{
	qreal bottom, top, left, right;

	bottom = _trackPaths.at(0)->boundingRect().bottom();
	top = _trackPaths.at(0)->boundingRect().top();
	left = _trackPaths.at(0)->boundingRect().left();
	right = _trackPaths.at(0)->boundingRect().right();

	for (int i = 1; i < _trackPaths.size(); i++) {
		bottom = qMax(bottom, _trackPaths.at(i)->boundingRect().bottom());
		top = qMin(top, _trackPaths.at(i)->boundingRect().top());
		right = qMax(right, _trackPaths.at(i)->boundingRect().right());
		left = qMin(left, _trackPaths.at(i)->boundingRect().left());
	}

	QRectF br(QPointF(left, top), QPointF(right, bottom));
	QPointF sc(br.width() / (viewport()->width() - MARGIN/2),
	  br.height() / (viewport()->height() - MARGIN/2));

	return qMax(sc.x(), sc.y());
}

qreal Track::mapScale() const
{
	return ((360.0/(qreal)(1<<_zoom))/(qreal)TILE_SIZE);
}

void Track::rescale(qreal scale)
{
	for (int i = 0; i < _trackPaths.size(); i++) {
		_markers.at(i)->setScale(scale);
		_trackPaths.at(i)->setScale(1.0/scale);

		QPen pen(_trackPaths.at(i)->pen());
		pen.setWidthF(TRACK_WIDTH * scale);
		_trackPaths.at(i)->setPen(pen);
	}

	QHash<Entry, POIItem*>::const_iterator it;
	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->setPos(QPointF(it.value()->entry().coordinates.x()
		  * 1.0/scale, -it.value()->entry().coordinates.y() * 1.0/scale));

	_scale = scale;
}

void Track::loadPOI(const POI &poi)
{
	QHash<Entry, POIItem*>::const_iterator it,jt;

	for (int i = 0; i < _tracks.size(); i++) {
		QVector<Entry> p = poi.points(_tracks.at(i));

		for (int i = 0; i < p.size(); i++) {
			if (_pois.contains(p.at(i)))
				continue;

			POIItem *pi = new POIItem(p.at(i));
			pi->setPos(p.at(i).coordinates.x() * 1.0/_scale,
			  -p.at(i).coordinates.y() * 1.0/_scale);
			pi->setZValue(1);
			_scene->addItem(pi);

			_pois.insert(p.at(i), pi);
		}
	}

	for (it = _pois.constBegin(); it != _pois.constEnd(); it++) {
		for (jt = _pois.constBegin(); jt != _pois.constEnd(); jt++) {
			if (it != jt && it.value()->isVisible() && jt.value()->isVisible()
			  && it.value()->collidesWithItem(jt.value()))
				jt.value()->hide();
		}
	}

	_scene->setSceneRect(_scene->itemsBoundingRect());
}

void Track::setMap(Map *map)
{
	_map = map;
	if (_map)
		connect(_map, SIGNAL(loaded()), this, SLOT(redraw()));
	resetCachedContent();
}

void Track::redraw()
{
	resetCachedContent();
}

void Track::wheelEvent(QWheelEvent *event)
{
	_zoom = (event->delta() > 0) ?
		qMin(_zoom + 1, ZOOM_MAX) : qMax(_zoom - 1, ZOOM_MIN);

	rescale(mapScale());
	_scene->setSceneRect(_scene->itemsBoundingRect());
	resetCachedContent();
}

void Track::showMarkers(bool show)
{
	for (int i = 0; i < _markers.size(); i++)
		_markers.at(i)->setVisible(show);
}

void Track::setTrackLineWidth(qreal width)
{
	for (int i = 0; i < _trackPaths.size(); i++) {
		QPen pen(_trackPaths.at(i)->pen());
		pen.setWidthF(width);
		_trackPaths.at(i)->setPen(pen);
	}
}

void Track::plot(QPainter *painter, const QRectF &target)
{
	QRectF orig = sceneRect();
	QRectF adj;
	qreal ratio, diff;

	if (target.width()/target.height() > orig.width()/orig.height()) {
		ratio = target.width()/target.height();
		diff = qAbs((orig.height() * ratio) - orig.width());
		adj = orig.adjusted(-diff/2, 0, diff/2, 0);
	} else {
		ratio = target.height()/target.width();
		diff = fabs((orig.width() * ratio) - orig.height());
		adj = orig.adjusted(0, -diff/2, 0, diff/2);
	}

	showMarkers(false);
	setTrackLineWidth(0);
	_scene->render(painter, target, adj, Qt::KeepAspectRatioByExpanding);
	setTrackLineWidth(TRACK_WIDTH * _scale);
	showMarkers(true);
}

enum QPrinter::Orientation Track::orientation() const
{
	return (sceneRect().width() > sceneRect().height())
		? QPrinter::Landscape : QPrinter::Portrait;
}

void Track::clearPOI()
{
	QHash<Entry, POIItem*>::const_iterator it;

	for (it = _pois.constBegin(); it != _pois.constEnd(); it++) {
		_scene->removeItem(it.value());
		delete it.value();
	}

	_pois.clear();
}

void Track::clear()
{
	_pois.clear();
	_tracks.clear();
	_trackPaths.clear();
	_markers.clear();
	_scene->clear();
	_colorShop.reset();

	_maxLen = 0;

	_scene->setSceneRect(0, 0, 0, 0);
}

void Track::movePositionMarker(qreal val)
{
	for (int i = 0; i < _trackPaths.size(); i++) {
		qreal f = _maxLen / _trackPaths.at(i)->path().length();
		QPointF pos = _trackPaths.at(i)->path().pointAtPercent(qMin(val * f,
		  1.0));
		_markers.at(i)->setPos(pos);
	}
}

void Track::drawBackground(QPainter *painter, const QRectF &rect)
{
	if (_tracks.isEmpty() || !_map) {
		painter->fillRect(rect, Qt::white);
		return;
	}

	painter->setWorldMatrixEnabled(false);

	QRectF rr(rect.topLeft() * _scale, rect.size());
	QPoint tile = mercator2tile(QPointF(rr.topLeft().x(), -rr.topLeft().y()),
	  _zoom);
	QPointF tm = tile2mercator(tile, _zoom);
	QPoint tl = mapFromScene(QPointF(tm.x() / _scale, -tm.y() / _scale));

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
