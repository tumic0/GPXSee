#include <QImageReader>
#include "tooltip.h"


#define THUMBNAIL_MAX_SIZE 240

void ToolTip::insert(const QString &key, const QString &value)
{
	_list.append(KV(key, value));
}

QString ToolTip::toString() const
{
	QString html;

	if (_img.isValid()) {
		int width, height;
		if (_img.size().width() > _img.size().height()) {
			width = qMin(_img.size().width(), THUMBNAIL_MAX_SIZE);
			qreal ratio = _img.size().width() / (qreal)_img.size().height();
			height = (int)(width / ratio);
		} else {
			height = qMin(_img.size().height(), THUMBNAIL_MAX_SIZE);
			qreal ratio = _img.size().height() / (qreal)_img.size().width();
			width = (int)(height / ratio);
		}

		html += "<div align=\"center\">";
		html += QString("<img src=\"file:%0\" width=\"%1\" height=\"%2\"/>")
		  .arg(_img.path(), QString::number(width), QString::number(height));
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
