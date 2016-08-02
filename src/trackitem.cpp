#include <QApplication>
#include <QCursor>
#include <QPen>
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
	_shape = s.createStroke(path().simplified());
}

TrackItem::TrackItem(const Track &track, QGraphicsItem *parent)
  : QGraphicsPathItem(parent)
{
	QVector<QPointF> t;
	QPainterPath path;

	track.track(t);
	Q_ASSERT(t.count() >= 2);

	const QPointF &p = t.at(0);
	path.moveTo(ll2mercator(QPointF(p.x(), -p.y())));
	for (int i = 1; i < t.size(); i++) {
		const QPointF &p = t.at(i);
		path.lineTo(ll2mercator(QPointF(p.x(), -p.y())));
	}

	_units = Metric;
	_date = track.date();
	_distance = track.distance();
	_time = track.time();

	setPath(path);
	setToolTip(toolTip());
	setCursor(Qt::ArrowCursor);

	updateShape();

	QBrush brush(Qt::SolidPattern);
	QPen pen(brush, TRACK_WIDTH);
	setPen(pen);
}

void TrackItem::setScale(qreal scale)
{
	QGraphicsPathItem::setScale(scale);
	updateShape();

	QPen p(pen());
	p.setWidthF(TRACK_WIDTH * 1.0/scale);
	setPen(p);
}

void TrackItem::setColor(const QColor &color)
{
	QPen p(pen());
	p.setColor(color);
	setPen(p);
}

void TrackItem::setUnits(enum Units units)
{
	_units = units;
	setToolTip(toolTip());
}
