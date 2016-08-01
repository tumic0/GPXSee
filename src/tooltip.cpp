#include "tooltip.h"

ToolTip::ToolTip()
{
}

void ToolTip::insert(const QString &key, const QString &value)
{
	KV kv(key, value);
	int i;

	if ((i = _list.indexOf(kv)) < 0)
		_list.append(kv);
	else
		_list[i] = kv;
}

QString ToolTip::toString()
{
	QString ret;

	for (int i = 0; i < _list.count(); i++)
		ret += "<b>" + _list.at(i).key + ":</b> " + _list.at(i).value + "<br/>";

	return ret;
}
