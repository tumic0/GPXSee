#include <cmath>
#include <QCursor>
#include <QPainter>
#include "common/greatcircle.h"
#include "map/map.h"
#include "font.h"
#include "pathitem.h"


#define GEOGRAPHICAL_MILE 1855.3248

static inline bool isValid(const QPointF &p)
{
	return (!std::isnan(p.x()) && !std::isnan(p.y()));
}

static inline unsigned segments(qreal distance)
{
	return ceil(distance / GEOGRAPHICAL_MILE);
}

static QFont font()
{
	QFont font;
	font.setPixelSize(10);
	font.setFamily(FONT_FAMILY);
	font.setBold(true);

	return font;
}

QFont PathItem::_font = font();

PathItem::PathItem(const Path &path, Map *map, QGraphicsItem *parent)
  : QGraphicsObject(parent), _path(path), _map(map)
{
	Q_ASSERT(_path.isValid());

	_units = Metric;
	_digitalZoom = 0;
	_width = 3;
	QBrush brush(Qt::SolidPattern);
	_pen = QPen(brush, _width);
	_showMarker = true;
	_showTicks = false;

	updatePainterPath();
	updateShape();
	updateTickRects();

	_markerDistance = _path.first().first().distance();
	_marker = new MarkerItem(this);
	_marker->setPos(position(_markerDistance));

	setCursor(Qt::ArrowCursor);
	setAcceptHoverEvents(true);
}

void PathItem::updateShape()
{
	QPainterPathStroker s;
	s.setWidth((_width + 1) * pow(2, -_digitalZoom));
	_shape = s.createStroke(_painterPath);
}

void PathItem::addSegment(const Coordinates &c1, const Coordinates &c2)
{
	if (fabs(c1.lon() - c2.lon()) > 180.0) {
		// Split segment on date line crossing
		QPointF p;

		if (c2.lon() < 0) {
			QLineF l(QPointF(c1.lon(), c1.lat()), QPointF(c2.lon() + 360,
			  c2.lat()));
			QLineF dl(QPointF(180, -90), QPointF(180, 90));
			l.intersect(dl, &p);
			_painterPath.lineTo(_map->ll2xy(Coordinates(180, p.y())));
			_painterPath.moveTo(_map->ll2xy(Coordinates(-180, p.y())));
		} else {
			QLineF l(QPointF(c1.lon(), c1.lat()), QPointF(c2.lon() - 360,
			  c2.lat()));
			QLineF dl(QPointF(-180, -90), QPointF(-180, 90));
			l.intersect(dl, &p);
			_painterPath.lineTo(_map->ll2xy(Coordinates(-180, p.y())));
			_painterPath.moveTo(_map->ll2xy(Coordinates(180, p.y())));
		}
		_painterPath.lineTo(_map->ll2xy(c2));
	} else
		_painterPath.lineTo(_map->ll2xy(c2));
}

void PathItem::updatePainterPath()
{
	_painterPath = QPainterPath();

	for (int i = 0; i < _path.size(); i++) {
		const PathSegment &segment = _path.at(i);
		_painterPath.moveTo(_map->ll2xy(segment.first().coordinates()));

		for (int j = 1; j < segment.size(); j++) {
			const PathPoint &p1 = segment.at(j-1);
			const PathPoint &p2 = segment.at(j);
			unsigned n = segments(p2.distance() - p1.distance());

			if (n > 1) {
				GreatCircle gc(p1.coordinates(), p2.coordinates());
				Coordinates last = p1.coordinates();

				for (unsigned k = 1; k <= n; k++) {
					Coordinates c(gc.pointAt(k/(double)n));
					addSegment(last, c);
					last = c;
				}
			} else
				addSegment(p1.coordinates(), p2.coordinates());
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

	if (_showTicks) {
		painter->setFont(_font);
		painter->setRenderHint(QPainter::Antialiasing, false);

		for (int i = 1; i < _tickCount; i++) {
			QPoint pos(position(i * _tickSize * xInM()).toPoint());
			QPointF arrow[3] = {QPointF(pos.x() - 0.5, pos.y()),
			  QPointF(pos.x() + 2.5, pos.y() - 3),
			  QPointF(pos.x() - 3.5, pos.y() - 3)};
			QString val(QString::number(i * _tickSize));

			QRectF br(_tickRect);
			br.moveCenter(QPointF(pos.x() - 0.5, pos.y() - br.height()/2.0
			  - 2.5));
			painter->setPen(Qt::white);
			painter->setBrush(_pen.color());
			painter->drawPolygon(arrow, 3);
			painter->drawRoundedRect(br, 1.5, 1.5);
			painter->drawText(br, Qt::AlignCenter, val);
		}
	}

/*
	painter->setBrush(Qt::NoBrush);
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
	updateTickRects();

	QPointF pos = position(_markerDistance);
	if (isValid(pos))
		_marker->setPos(pos);
}

void PathItem::setColor(const QColor &color)
{
	if (_pen.color() == color)
		return;

	_pen.setColor(color);
	update();
}

void PathItem::setWidth(qreal width)
{
	if (_width == width)
		return;

	prepareGeometryChange();

	_width = width;
	_pen.setWidthF(_width * pow(2, -_digitalZoom));

	updateShape();
}

void PathItem::setStyle(Qt::PenStyle style)
{
	if (_pen.style() == style)
		return;

	_pen.setStyle(style);
	update();
}

void PathItem::setDigitalZoom(int zoom)
{
	if (_digitalZoom == zoom)
		return;

	prepareGeometryChange();

	_digitalZoom = zoom;
	_pen.setWidthF(_width * pow(2, -_digitalZoom));
	_marker->setScale(pow(2, -_digitalZoom));

	updateShape();
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

	unsigned n = segments(p2 - p1);
	if (n > 1) {
		GreatCircle gc(c1, c2);

		// Great circle point
		double f = (x - p1) / (p2 - p1);
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
		return l.pointAt((x - p1) / (p2 - p1));
	}
}

void PathItem::moveMarker(qreal distance)
{
	_markerDistance = distance;
	QPointF pos(position(distance));

	if (isValid(pos)) {
		_marker->setVisible(_showMarker);
		_marker->setPos(pos);
	} else
		_marker->setVisible(false);
}

void PathItem::setMarkerColor(const QColor &color)
{
	_marker->setColor(color);
}

void PathItem::hover(bool hover)
{
	if (hover) {
		_pen.setWidth((_width + 1) * pow(2, -_digitalZoom));
		setZValue(zValue() + 1.0);
	} else {
		_pen.setWidth(_width * pow(2, -_digitalZoom));
		setZValue(zValue() - 1.0);
	}

	update();
}

void PathItem::showMarker(bool show)
{
	if (_showMarker == show)
		return;

	_showMarker = show;
	_marker->setVisible(show && isValid(position(_markerDistance)));
}

qreal PathItem::xInM() const
{
	return (_units == Nautical) ? NMIINM : (_units == Imperial) ? MIINM : KMINM;
}

unsigned PathItem::tickSize() const
{
	qreal r = _path.last().last().distance() / xInM();

	if (r < 1)
		return 0;
	else if (r < 15)
		return 1;
	else if (r < 50)
		return 5;
	else if (r < 150)
		return 10;
	else if (r < 500)
		return 50;
	else if (r < 1500)
		return 100;
	else if (r < 5000)
		return 500;
	else
		return 1000;
}

void PathItem::updateTickRects()
{
	_tickSize = tickSize();
	qreal f = xInM();
	_tickCount = (int)_path.last().last().distance() / (_tickSize * f) + 1;

	QFontMetrics fm(_font);
	_tickRect = fm.boundingRect(QRect(), Qt::AlignCenter,
	  QString::number(qMax(_tickSize * (_tickCount - 1), 10)))
	  .adjusted(-2, 0, 2, 0);

	_tickBoundingRect = QRectF();
	for (int i = 1; i < _tickCount; i++) {
		QPoint pos(position(i * _tickSize * xInM()).toPoint());
		QRectF br(_tickRect);
		br.moveCenter(QPointF(pos.x() - 0.5, pos.y() - br.height()/2.0
		  - 2.5));
		_tickBoundingRect |= br;
	}
}

void PathItem::showTicks(bool show)
{
	if (_showTicks == show)
		return;

	prepareGeometryChange();
	_showTicks = show;
}

void PathItem::setUnits(Units units)
{
	if (_units == units)
		return;

	prepareGeometryChange();
	_units = units;
	updateTickRects();
}

void PathItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_pen.setWidthF((_width + 1) * pow(2, -_digitalZoom));
	setZValue(zValue() + 1.0);
	update();

	emit selected(true);
}

void PathItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_pen.setWidthF(_width * pow(2, -_digitalZoom));
	setZValue(zValue() - 1.0);
	update();

	emit selected(false);
}

QRectF PathItem::boundingRect() const
{
	return _showTicks
	  ? _shape.boundingRect() | _tickBoundingRect
	  : _shape.boundingRect();
}
