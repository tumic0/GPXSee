#include <QCursor>
#include <QPen>
#include "ll.h"
#include "misc.h"
#include "trackitem.h"


#define TRACK_WIDTH     3

QString TrackItem::toolTip()
{
	QString date = _date.date().toString(Qt::SystemLocaleShortDate);

	return "<b>" + QObject::tr("Date:") + "</b> " + date + "<br><b>"
	  + QObject::tr("Distance:") + "</b> " + distance(_distance, _units)
	  + "<br><b>" + QObject::tr("Time:") + "</b> " + timeSpan(_time);
}

void TrackItem::updateShape()
{
	QPainterPathStroker s;
	s.setWidth(TRACK_WIDTH * 1.0/scale());
	_shape = s.createStroke(path().simplified());
}

TrackItem::TrackItem(const Track &track)
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
	pen.setCosmetic(true);
	setPen(pen);
}

void TrackItem::setScale(qreal scale)
{
	QGraphicsPathItem::setScale(scale);
	updateShape();
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
