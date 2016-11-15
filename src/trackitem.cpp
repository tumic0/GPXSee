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
	  Format::distance(_distance.last(), _units));
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
	const Path &path = track.path();
	Q_ASSERT(path.count() >= 2);

	p = path.first().coordinates().toMercator();
	_path.moveTo(QPointF(p.x(), -p.y()));
	_distance.append(path.first().distance());
	for (int i = 1; i < path.size(); i++) {
		if (path.at(i).coordinates() == path.at(i-1).coordinates())
			continue;
		p = path.at(i).coordinates().toMercator();
		_path.lineTo(QPointF(p.x(), -p.y()));
		_distance.append(path.at(i).distance());
	}

	updateShape();

	_name = track.name();
	_desc = track.description();
	_date = track.date();
	_time = track.time();

	_marker->setPos(_path.elementAt(0));

	setToolTip(toolTip());
}

void TrackItem::setUnits(enum Units units)
{
	PathItem::setUnits(units);
	setToolTip(toolTip());
}
