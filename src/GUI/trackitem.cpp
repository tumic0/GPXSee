#include <QPainter>
#include "map/map.h"
#include "format.h"
#include "tooltip.h"
#include "trackitem.h"


QString TrackItem::info() const
{
	ToolTip tt;

	if (!_name.isEmpty())
		tt.insert(tr("Name"), _name);
	if (!_desc.isEmpty())
		tt.insert(tr("Description"), _desc);
	if (!_comment.isEmpty() && _comment != _desc)
		tt.insert(tr("Comment"), _comment);
	tt.insert(tr("Distance"), Format::distance(path().last().last().distance(),
	  _units));
	if  (_time > 0)
		tt.insert(tr("Total time"), Format::timeSpan(_time));
	if  (_movingTime > 0)
		tt.insert(tr("Moving time"), Format::timeSpan(_movingTime));
	if (!_date.isNull())
		tt.insert(tr("Date"),
#ifdef ENABLE_TIMEZONES
		  _date.toTimeZone(_timeZone)
#else // ENABLE_TIMEZONES
		  _date
#endif // ENABLE_TIMEZONES
		  .toString(Qt::SystemLocaleShortDate));
	if (!_links.isEmpty()) {
		QString links;
		for (int i = 0; i < _links.size(); i++) {
			const Link &link = _links.at(i);
			links.append(QString("<a href=\"%0\">%1</a>").arg(link.URL(),
			  link.text().isEmpty() ? link.URL() : link.text()));
			if (i != _links.size() - 1)
				links.append("<br/>");
		}
		tt.insert(tr("Links"), links);
	}

	return tt.toString();
}

TrackItem::TrackItem(const Track &track, Map *map, QGraphicsItem *parent)
  : PathItem(track.path(), map, parent)
{
	_name = track.name();
	_desc = track.description();
	_comment = track.comment();
	_links = track.links();
	_date = track.date();
	_time = track.time();
	_movingTime = track.movingTime();
}
