#include "popup.h"
#include "tooltip.h"

void ToolTip::insert(const QString &key, const QString &value)
{
	_list.append(KV<QString, QString>(key, value));
}

QString ToolTip::toString() const
{
	QString html;

	if (!_list.isEmpty()) {
		html += "<table>";
		for (int i = 0; i < _list.count(); i++)
			html += "<tr><td align=\"right\"><b>" + _list.at(i).key()
			  + ":&nbsp;</b></td><td>" + _list.at(i).value() + "</td></tr>";
		html += "</table>";
	}

	return html;
}
