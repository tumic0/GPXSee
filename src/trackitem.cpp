#include <QPainter>
#include "format.h"
#include "tooltip.h"
#include "map.h"
#include "trackitem.h"


QString TrackItem::toolTip(Units units)
{
	ToolTip tt;

	if (!_name.isEmpty())
		tt.insert(tr("Name"), _name);
	if (!_desc.isEmpty())
		tt.insert(tr("Description"), _desc);
	tt.insert(tr("Distance"), Format::distance(_path.last().distance(), units));
	if  (_time > 0)
		tt.insert(tr("Total time"), Format::timeSpan(_time));
	if  (_movingTime > 0)
		tt.insert(tr("Moving time"), Format::timeSpan(_movingTime));
	if (!_date.isNull())
		tt.insert(tr("Date"), _date.toString(Qt::SystemLocaleShortDate));

	return tt.toString();
}

TrackItem::TrackItem(const Track &track, Map *map, QGraphicsItem *parent)
  : PathItem(track.path(), map, parent)
{
	_name = track.name();
	_desc = track.description();
	_date = track.date();
	_time = track.time();
	_movingTime = track.movingTime();

	setToolTip(toolTip(Metric));
}

void TrackItem::setUnits(Units units)
{
	setToolTip(toolTip(units));
}
