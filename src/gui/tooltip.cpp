#include "tooltip.h"

void ToolTip::insert(const QString &key, const QString &value)
{
	QPair<QString, QString> entry(key, value);
	_list.append(entry);
}

QString ToolTip::toString()
{
	QString ret = "<table>";

	for (int i = 0; i < _list.count(); i++)
		ret += "<tr><td align=\"right\"><b>" + _list.at(i).first
		  + ":&nbsp;</b></td><td>" + _list.at(i).second + "</td></tr>";

	ret += "</table>";

	return ret;
}
