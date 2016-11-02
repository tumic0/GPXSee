#include <QApplication>
#include <QCursor>
#include <QPainter>
#include "format.h"
#include "tooltip.h"
#include "trackitem.h"


QString TrackItem::toolTip()
{
	ToolTip tt;

	if (!_name.isEmpty())
		tt.insert(qApp->translate("TrackItem", "Name"), _name);
	if (!_desc.isEmpty())
		tt.insert(qApp->translate("TrackItem", "Description"), _desc);
	tt.insert(qApp->translate("TrackItem", "Distance"),
	  Format::distance(_distance, _units));
	if  (_time > 0)
		tt.insert(qApp->translate("TrackItem", "Time"),
		  Format::timeSpan(_time));
	if (!_date.isNull())
		tt.insert(qApp->translate("TrackItem", "Date"),
		  _date.toString(Qt::SystemLocaleShortDate));

	return tt.toString();
}

TrackItem::TrackItem(const Track &track, QGraphicsItem *parent)
  : PathItem(parent)
{
	QPointF p;
	const TrackData &t = track.track();
	Q_ASSERT(t.count() >= 2);

	p = t.first().coordinates().toMercator();
	_path.moveTo(QPointF(p.x(), -p.y()));
	for (int i = 1; i < t.size(); i++) {
		p = t.at(i).coordinates().toMercator();
		_path.lineTo(QPointF(p.x(), -p.y()));
	}

	updateShape();

	_name = t.name();
	_desc = t.description();
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
