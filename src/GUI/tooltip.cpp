#include "popup.h"
#include "tooltip.h"

static QSize thumbnailSize(const ImageInfo &img, int limit)
{
	int width, height;
	if (img.size().width() > img.size().height()) {
		width = qMin(img.size().width(), limit);
		qreal ratio = img.size().width() / (qreal)img.size().height();
		height = (int)(width / ratio);
	} else {
		height = qMin(img.size().height(), limit);
		qreal ratio = img.size().height() / (qreal)img.size().width();
		width = (int)(height / ratio);
	}

	return QSize(width, height);
}

void ToolTip::insert(const QString &key, const QString &value)
{
	_list.append(KV<QString, QString>(key, value));
}

QString ToolTip::toString() const
{
	QString html;

	if (_images.size()) {
		html = "<div align=\"center\">";
		for (int i = 0; i < _images.size(); i++) {
			const ImageInfo &img = _images.at(i);
			QSize size(thumbnailSize(img, qMin(960/_images.size(), 240)));

			html += QString("<a href=\"file:%0\">"
			  "<img src=\"%0\" width=\"%1\" height=\"%2\"/></a>")
			  .arg(img.path(), QString::number(size.width()),
			  QString::number(size.height()));
		}
		html += "</div>";
	}

	if (!_list.isEmpty()) {
		html += "<table>";
		for (int i = 0; i < _list.count(); i++)
			html += "<tr><td align=\"right\"><b>" + _list.at(i).key()
			  + ":&nbsp;</b></td><td>" + _list.at(i).value() + "</td></tr>";
		html += "</table>";
	}

	return html;
}
