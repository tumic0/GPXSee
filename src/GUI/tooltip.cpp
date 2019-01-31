#include "tooltip.h"

void ToolTip::insert(const QString &key, const QString &value)
{
	_list.append(KV(key, value));
}

QString ToolTip::toString()
{
	if (_list.isEmpty())
		return QString();

	QString ret = "<table>";

	for (int i = 0; i < _list.count(); i++)
		ret += "<tr><td align=\"right\"><b>" + _list.at(i).key()
		  + ":&nbsp;</b></td><td>" + _list.at(i).value() + "</td></tr>";

	ret += "</table>";

	return ret;
}
