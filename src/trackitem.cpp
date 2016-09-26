#include <QApplication>
#include <QCursor>
#include <QPainter>
#include "ll.h"
#include "misc.h"
#include "tooltip.h"
#include "trackitem.h"


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

TrackItem::TrackItem(const Track &track, QGraphicsItem *parent)
  : PathItem(parent)
{
	const QVector<Trackpoint> &t = track.track();
	Q_ASSERT(t.count() >= 2);

	const QPointF &p = t.at(0).coordinates();
	_path.moveTo(ll2mercator(QPointF(p.x(), -p.y())));
	for (int i = 1; i < t.size(); i++) {
		const QPointF &p = t.at(i).coordinates();
		_path.lineTo(ll2mercator(QPointF(p.x(), -p.y())));
	}

	updateShape();

	_date = track.date();
	_distance = track.distance();
	_time = track.time();

	_marker->setPos(_path.pointAtPercent(0));

	setToolTip(toolTip());
}

void TrackItem::setUnits(enum Units units)
{
	PathItem::setUnits(units);
	setToolTip(toolTip());
}
