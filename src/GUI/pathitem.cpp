#include <cmath>
#include <QCursor>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include "common/greatcircle.h"
#include "map/map.h"
#include "pathtickitem.h"
#include "popup.h"
#include "graphitem.h"
#include "markeritem.h"
#include "pathitem.h"

#define GEOGRAPHICAL_MILE 1855.3248

Units PathItem::_units = Metric;
QTimeZone PathItem::_timeZone = QTimeZone::utc();

static inline bool isValid(const QPointF &p)
{
	return (!std::isnan(p.x()) && !std::isnan(p.y()));
}

static inline unsigned segments(qreal distance)
{
	return ceil(distance / GEOGRAPHICAL_MILE);
}

PathItem::PathItem(const Path &path, Map *map, QGraphicsItem *parent)
  : GraphicsItem(parent), _path(path), _map(map), _graph(0)
{
	Q_ASSERT(_path.isValid());

	_digitalZoom = 0;
	_width = 3;
	_color = Qt::black;
	_penStyle = Qt::SolidLine;
	_showMarker = true;
	_showTicks = false;
	_markerInfoType = MarkerInfoItem::None;

	_pen = QPen(color(), width());

	updatePainterPath();
	updateShape();
	updateTicks();

	_markerDistance = _path.first().first().distance();
	_marker = new MarkerItem(this);
	_marker->setZValue(1);
	_marker->setPos(position(_markerDistance));
	_markerInfo = new MarkerInfoItem(_marker);
	_markerInfo->setVisible(false);

	setCursor(Qt::ArrowCursor);
	setAcceptHoverEvents(true);
}

void PathItem::updateShape()
{
	QPainterPathStroker s;
	s.setWidth((_width + 1) * pow(2, -_digitalZoom));
	_shape = s.createStroke(_painterPath);
}

bool PathItem::addSegment(const Coordinates &c1, const Coordinates &c2)
{
	if (fabs(c1.lon() - c2.lon()) > 180.0) {
		// Split segment on date line crossing
		QPointF p;

		if (c2.lon() < 0) {
			QLineF l(QPointF(c1.lon(), c1.lat()), QPointF(c2.lon() + 360,
			  c2.lat()));
			QLineF dl(QPointF(180, -90), QPointF(180, 90));
			l.intersects(dl, &p);
			_painterPath.lineTo(_map->ll2xy(Coordinates(180, p.y())));
			_painterPath.moveTo(_map->ll2xy(Coordinates(-180, p.y())));
		} else {
			QLineF l(QPointF(c1.lon(), c1.lat()), QPointF(c2.lon() - 360,
			  c2.lat()));
			QLineF dl(QPointF(-180, -90), QPointF(-180, 90));
			l.intersects(dl, &p);
			_painterPath.lineTo(_map->ll2xy(Coordinates(-180, p.y())));
			_painterPath.moveTo(_map->ll2xy(Coordinates(180, p.y())));
		}
		_painterPath.lineTo(_map->ll2xy(c2));

		return true;
	} else {
		QPointF p(_map->ll2xy(c2));
		const QPainterPath::Element &e = _painterPath.elementAt(
		  _painterPath.elementCount() - 1);
		qreal dx = qAbs(p.x() - e.x);
		qreal dy = qAbs(p.y() - e.y);

		if (dx >= 1.0 || dy >= 1.0) {
			_painterPath.lineTo(p);
			return true;
		} else
			return false;
	}
}

void PathItem::updatePainterPath()
{
	_painterPath = QPainterPath();

	for (int i = 0; i < _path.size(); i++) {
		const PathSegment &segment = _path.at(i);
		const PathPoint *p1 = &segment.first();

		_painterPath.moveTo(_map->ll2xy(p1->coordinates()));

		for (int j = 1; j < segment.size(); j++) {
			const PathPoint *p2 = &segment.at(j);
			double dist = p2->distance() - p1->distance();

			if (dist > GEOGRAPHICAL_MILE) {
				GreatCircle gc(p1->coordinates(), p2->coordinates());
				Coordinates last(p1->coordinates());
				unsigned n = segments(dist);

				for (unsigned k = 1; k <= n; k++) {
					Coordinates c(gc.pointAt(k/(double)n));
					addSegment(last, c);
					last = c;
				}
				p1 = p2;
			} else {
				if (addSegment(p1->coordinates(), p2->coordinates()))
					p1 = p2;
			}
		}
	}
}

void PathItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	painter->setPen(_pen);
	painter->drawPath(_painterPath);

/*
	painter->setPen(Qt::red);
	painter->drawRect(boundingRect());
*/
}

void PathItem::setMap(Map *map)
{
	prepareGeometryChange();

	_map = map;

	updatePainterPath();
	updateShape();
	updateTicks();

	QPointF pos = position(_markerDistance);
	if (isValid(pos))
		_marker->setPos(pos);
}

const QColor &PathItem::color() const
{
	return (_useStyle && _path.style().color().isValid())
	  ? _path.style().color() : _color;
}

void PathItem::setColor(const QColor &color)
{
	_color = color;
	updateColor();
}

void PathItem::updateColor()
{
	const QColor &c(color());

	_pen.setColor(c);

	for (int i = 0; i < _ticks.size(); i++)
		_ticks[i]->setColor(c);

	update();
}

qreal PathItem::width() const
{
	return (_useStyle && _path.style().width() > 0)
	  ? _path.style().width() : _width;
}

void PathItem::setWidth(qreal width)
{
	_width = width;
	updateWidth();
}

void PathItem::updateWidth()
{
	prepareGeometryChange();

	_pen.setWidthF(width() * pow(2, -_digitalZoom));

	updateShape();
}

Qt::PenStyle PathItem::penStyle() const
{
	return (_useStyle && _path.style().style() != Qt::NoPen)
	  ? _path.style().style() : _penStyle;
}

void PathItem::setPenStyle(Qt::PenStyle style)
{
	_penStyle = style;
	updatePenStyle();
}

void PathItem::updatePenStyle()
{
	_pen.setStyle(penStyle());
	update();
}

void PathItem::setDigitalZoom(int zoom)
{
	if (_digitalZoom == zoom)
		return;

	prepareGeometryChange();

	_digitalZoom = zoom;
	_pen.setWidthF(width() * pow(2, -_digitalZoom));
	_marker->setScale(pow(2, -_digitalZoom));
	for (int i = 0; i < _ticks.size(); i++)
		_ticks.at(i)->setDigitalZoom(zoom);

	updateShape();
}

void PathItem::updateStyle()
{
	updateColor();
	updateWidth();
	updatePenStyle();

	for (int i = 0; i < _graphs.size(); i++) {
		GraphItem *graph = _graphs.at(i);
		if (graph)
			graph->updateStyle();
	}
}

const PathSegment *PathItem::segment(qreal x) const
{
	for (int i = 0; i < _path.size(); i++)
		if (x <= _path.at(i).last().distance())
			return &(_path.at(i));

	return 0;
}

QPointF PathItem::position(qreal x) const
{
	const PathSegment *seg = segment(x);
	if (!seg)
		return QPointF(NAN, NAN);

	int low = 0;
	int high = seg->count() - 1;
	int mid = 0;

	if (!(x >= seg->first().distance() && x <= seg->last().distance()))
		return QPointF(NAN, NAN);

	while (low <= high) {
		mid = low + ((high - low) / 2);
		qreal val = seg->at(mid).distance();
		if (val > x)
			high = mid - 1;
		else if (val < x)
			low = mid + 1;
		else
			return _map->ll2xy(seg->at(mid).coordinates());
	}

	Coordinates c1, c2;
	qreal p1, p2;

	if (seg->at(mid).distance() < x) {
		c1 = seg->at(mid).coordinates(); c2 = seg->at(mid+1).coordinates();
		p1 = seg->at(mid).distance(); p2 = seg->at(mid+1).distance();
	} else {
		c1 = seg->at(mid-1).coordinates(); c2 = seg->at(mid).coordinates();
		p1 = seg->at(mid-1).distance(); p2 = seg->at(mid).distance();
	}

	qreal dist = p2 - p1;

	if (dist > GEOGRAPHICAL_MILE) {
		GreatCircle gc(c1, c2);
		unsigned n = segments(dist);

		// Great circle point
		double f = (x - p1) / (dist);
		QPointF p(_map->ll2xy(gc.pointAt(f)));

		// Segment line of the great circle path
		double f1 = floor(n * f) / n;
		double f2 = ceil(n * f) / n;
		QLineF l(_map->ll2xy(gc.pointAt(f1)), _map->ll2xy(gc.pointAt(f2)));

		// Project the great circle point to the segment line
		QLineF u = l.unitVector();
		double lambda = (u.dx() * (p.x() - l.p1().x())) + (u.dy() * (p.y()
		  - l.p1().y()));
		return QPointF((u.dx() * lambda) + l.p1().x(), (u.dy() * lambda)
		  + l.p1().y());
	} else {
		QLineF l(_map->ll2xy(c1), _map->ll2xy(c2));
		return l.pointAt((x - p1) / (dist));
	}
}

void PathItem::setMarkerPosition(qreal pos)
{
	qreal distance = _graph
	  ? (_graph->graphType() == Time) ? _graph->distanceAtTime(pos) : pos
	  : NAN;

	_markerDistance = distance;
	QPointF pp(position(distance));

	if (isValid(pp)) {
		_marker->setVisible(_showMarker);
		_marker->setPos(pp);
		setMarkerInfo(pos);
	} else
		_marker->setVisible(false);
}

void PathItem::setMarkerInfo(qreal pos)
{
	if (_markerInfoType == MarkerInfoItem::Date) {
		QDateTime date;

		if (_graph) {
			qreal time = (_graph->graphType() == Time)
			  ? pos : _graph->timeAtDistance(pos);
			GraphItem::SegmentTime st(_graph->date(pos));
			if (st.date.isValid() && !std::isnan(time))
				date = st.date.addSecs(time - st.time);
		}

		if (date.isValid())
			_markerInfo->setDate(date.toTimeZone(_timeZone));
		else
			_markerInfo->setDate(QDateTime());
	} else if (_markerInfoType == MarkerInfoItem::Position)
		_markerInfo->setCoordinates(_map->xy2ll(_marker->pos()));
}

void PathItem::updateMarkerInfo()
{
	qreal pos = _graph ? (_graph->graphType() == Time)
	  ? _graph->timeAtDistance(_markerDistance) : _markerDistance : NAN;
	setMarkerInfo(pos);
}

void PathItem::setMarkerColor(const QColor &color)
{
	_marker->setColor(color);
	_markerInfo->setColor(color);
}

void PathItem::setMarkerBackgroundColor(const QColor &color)
{
	_markerInfo->setBackgroundColor(color);
}

void PathItem::drawMarkerBackground(bool draw)
{
	_markerInfo->drawBackground(draw);
}

void PathItem::hover(bool hover)
{
	if (hover) {
		_pen.setWidth((width() + 1) * pow(2, -_digitalZoom));
		setZValue(zValue() + 1.0);
	} else {
		_pen.setWidth(width() * pow(2, -_digitalZoom));
		setZValue(zValue() - 1.0);
	}

	update();
}

void PathItem::showMarker(bool show)
{
	if (_showMarker == show)
		return;

	_showMarker = show;
	updateMarkerInfo();
	_marker->setVisible(show && isValid(position(_markerDistance)));
}

void PathItem::showMarkerInfo(MarkerInfoItem::Type type)
{
	if (_markerInfoType == type)
		return;

	_markerInfoType = type;
	updateMarkerInfo();
	_markerInfo->setVisible(type > MarkerInfoItem::None);
}

qreal PathItem::xInM() const
{
	return (_units == Nautical) ? NMIINM : (_units == Imperial) ? MIINM : KMINM;
}

unsigned PathItem::tickSize() const
{
	qreal res = _map->resolution(sceneBoundingRect());

	if (res < 10)
		return 1;
	else if (res < 25)
		return 5;
	else if (res < 100)
		return 10;
	else if (res < 500)
		return 50;
	else if (res < 2000)
		return 100;
	else if (res < 10000)
		return 500;
	else if (res < 20000)
		return 1000;
	else
		return 5000;
}

void PathItem::updateTicks()
{
	qDeleteAll(_ticks);
	_ticks.clear();

	if (!_showTicks)
		return;

	int ts = tickSize();
	int tc = _path.last().last().distance() / (ts * xInM());
	QRect tr = PathTickItem::tickRect(ts * tc);

	_ticks.resize(tc);
	for (int i = 0; i < tc; i++) {
		_ticks[i] = new PathTickItem(tr, (i + 1) * ts, this);
		_ticks[i]->setPos(position((i + 1) * ts * xInM()));
		_ticks[i]->setColor(_pen.color());
	}
}

void PathItem::showTicks(bool show)
{
	if (_showTicks == show)
		return;

	prepareGeometryChange();
	_showTicks = show;
	updateTicks();
}

void PathItem::addGraph(GraphItem *graph)
{
	_graphs.append(graph);

	if (graph) {
		connect(this, &PathItem::selected, graph, &GraphItem::hover);
		connect(graph, &GraphItem::selected, this, &PathItem::hover);
		if (graph->secondaryGraph()) {
			connect(this, &PathItem::selected, graph->secondaryGraph(),
			  &GraphItem::hover);
			connect(graph->secondaryGraph(), &GraphItem::selected, this,
			  &PathItem::hover);
		}
	}
}

void PathItem::setGraph(int index)
{
	_graph = _graphs.at(index);
}

void PathItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_pen.setWidthF((width() + 1) * pow(2, -_digitalZoom));
	setZValue(zValue() + 1.0);
	update();

	emit selected(true);
}

void PathItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_pen.setWidthF(width() * pow(2, -_digitalZoom));
	setZValue(zValue() - 1.0);
	update();

	emit selected(false);
}

void PathItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	GraphicsScene *gs = dynamic_cast<GraphicsScene *>(scene());
	if (gs)
		Popup::show(event->screenPos(), info(gs->showExtendedInfo()),
		  event->widget());
	GraphicsItem::mousePressEvent(event);
}
