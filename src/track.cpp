#include <cmath>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QWheelEvent>
#include <QGraphicsEllipseItem>
#include "poiitem.h"
#include "markeritem.h"
#include "track.h"

#include <QDebug>


Track::Track(QWidget *parent)
	: QGraphicsView(parent)
{
	_scene = new QGraphicsScene(this);
	setScene(_scene);
	setResizeAnchor(QGraphicsView::AnchorViewCenter);

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


	gpx.track(track);

	if (track.size() < 2)
		return;

	_tracks.append(track);

	path.moveTo(track.at(0).x(), -track.at(0).y());
	for (int i = 1; i < track.size(); i++)
		path.lineTo(track.at(i).x(), -track.at(i).y());

	_maxLen = qMax(path.length(), _maxLen);

	for (int i = 0; i < _trackPaths.size(); i++) {
		_trackPaths.at(i)->resetTransform();
		_scene->removeItem(_markers.at(i));
	}

	QBrush brush(color, Qt::SolidPattern);
	QPen pen(brush, 0);
	pi = new QGraphicsPathItem(path);
	pi->setPen(pen);
	_scene->addItem(pi);
	_trackPaths.append(pi);

	QTransform t = transform();

	mi = new MarkerItem();
	mi->setPos(pi->path().pointAtPercent(0));
	_markers.append(mi);

	for (int i = 0; i < _trackPaths.size(); i++) {
		_markers.at(i)->setTransform(t);
		_scene->addItem(_markers.at(i));
	}

	_scene->setSceneRect(_scene->itemsBoundingRect());
	fitInView(_scene->sceneRect(), Qt::KeepAspectRatio);
}

QTransform Track::transform() const
{
	QPointF scale(_scene->itemsBoundingRect().width() / viewport()->width(),
	  _scene->itemsBoundingRect().height() / viewport()->height());
	QTransform transform;
	transform.scale(qMax(scale.x(), scale.y()), qMax(scale.x(), scale.y()));

	return transform;
}

void Track::loadPOI(const POI &poi)
{
	QHash<Entry, POIItem*>::const_iterator it,jt;

	for (int i = 0; i < _tracks.size(); i++) {
		QVector<Entry> p = poi.points(_tracks.at(i));

		for (int i = 0; i < p.size(); i++) {
			if (_pois.contains(p.at(i)))
				continue;

			POIItem *pi = new POIItem(p.at(i).description);
			pi->setPos(p.at(i).coordinates.x(), -p.at(i).coordinates.y());
			pi->setTransform(transform());
			pi->setZValue(1);
			_scene->addItem(pi);

			_pois.insert(p.at(i), pi);
		}
	}

	for (it = _pois.constBegin(); it != _pois.constEnd(); it++)
		it.value()->setTransform(transform());

	for (it = _pois.constBegin(); it != _pois.constEnd(); it++) {
		for (jt = _pois.constBegin(); jt != _pois.constEnd(); jt++) {
			if (it != jt && it.value()->isVisible() && jt.value()->isVisible()
			  && it.value()->collidesWithItem(jt.value()))
				jt.value()->hide();
		}
	}

	_scene->setSceneRect(_scene->itemsBoundingRect());
	fitInView(_scene->sceneRect(), Qt::KeepAspectRatio);
}

void Track::wheelEvent(QWheelEvent *event)
{
	float factor;

	factor = pow(2.0, -event->delta() / 400.0);
	scale(factor, factor);
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

	for (int i = 0; i < _markers.size(); i++)
		_markers.at(i)->setVisible(false);
	_scene->render(painter, target, adj, Qt::KeepAspectRatioByExpanding);
	for (int i = 0; i < _markers.size(); i++)
		_markers.at(i)->setVisible(true);
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
