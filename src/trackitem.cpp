#include <QApplication>
#include <QCursor>
#include <QPainter>
#include "ll.h"
#include "misc.h"
#include "tooltip.h"
#include "trackitem.h"


#define TRACK_WIDTH     3

QString TrackItem::toolTip()
{
	ToolTip tt;

	tt.insert(qApp->translate("TrackItem", "Distance"),
	  ::distance(_distance, _units));
	if  (_time > 0)
		tt.insert(qApp->translate("TrackItem", "Time"), ::timeSpan(_time));
	if (!_date.isNull())
		tt.insert(qApp->translate("TrackItem", "Date"),
		  _date.toString(Qt::SystemLocaleShortDate));

	return tt.toString();
}

void TrackItem::updateShape()
{
	QPainterPathStroker s;
	s.setWidth(TRACK_WIDTH * 1.0/scale());
	_shape = s.createStroke(_path.simplified());
}

TrackItem::TrackItem(const Track &track, QGraphicsItem *parent)
  : QGraphicsItem(parent)
{
	const QVector<QPointF> &t = track.track();
	Q_ASSERT(t.count() >= 2);

	const QPointF &p = t.at(0);
	_path.moveTo(ll2mercator(QPointF(p.x(), -p.y())));
	for (int i = 1; i < t.size(); i++) {
		const QPointF &p = t.at(i);
		_path.lineTo(ll2mercator(QPointF(p.x(), -p.y())));
	}

	_units = Metric;
	_date = track.date();
	_distance = track.distance();
	_time = track.time();

	setToolTip(toolTip());
	setCursor(Qt::ArrowCursor);

	updateShape();

	QBrush brush(Qt::SolidPattern);
	_pen = QPen(brush, TRACK_WIDTH);

	_marker = new MarkerItem(this);
	_marker->setPos(_path.pointAtPercent(0));
}

void TrackItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	painter->setPen(_pen);
	painter->drawPath(_path);

/*
	painter->setPen(Qt::red);
	painter->drawRect(boundingRect());
*/
}

void TrackItem::setScale(qreal scale)
{
	prepareGeometryChange();

	_pen.setWidthF(TRACK_WIDTH * 1.0/scale);
	QGraphicsItem::setScale(scale);
	_marker->setScale(1.0/scale);

	updateShape();
}

void TrackItem::setColor(const QColor &color)
{
	_pen.setColor(color);
	update();
}

void TrackItem::setUnits(enum Units units)
{
	_units = units;
	setToolTip(toolTip());
}

void TrackItem::moveMarker(qreal distance)
{
	if (distance > _distance)
		_marker->setVisible(false);
	else {
		_marker->setVisible(true);
		_marker->setPos(_path.pointAtPercent(distance / _distance));
	}
}
