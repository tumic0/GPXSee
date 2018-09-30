#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <QString>
#include <QList>
#include "common/kv.h"

class ToolTip
{
public:
	void insert(const QString &key, const QString &value);
	QString toString();

private:
	QList<KV> _list;
};

#endif // TOOLTIP_H
