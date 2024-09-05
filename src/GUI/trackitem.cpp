#include <QLocale>
#include "common/util.h"
#include "data/track.h"
#include "format.h"
#include "tooltip.h"
#include "trackitem.h"


ToolTip TrackItem::info(bool extended) const
{
	ToolTip tt;
	QLocale l;

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
	if (!_date.isNull()) {
		QDateTime date(_date.toTimeZone(_timeZone));
		tt.insert(tr("Date"), l.toString(date.date(), QLocale::ShortFormat)
		  + " " + date.time().toString("h:mm:ss"));
	}
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
#ifdef Q_OS_ANDROID
	Q_UNUSED(extended);
#else // Q_OS_ANDROID
	if (extended && !_file.isEmpty())
		tt.insert(tr("File"), QString("<a href=\"file:%1\">%2</a>")
		  .arg(_file, QFileInfo(_file).fileName()));
#endif // Q_OS_ANDROID

	return tt;
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
	_file = track.file();
}
