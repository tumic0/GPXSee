#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <QString>
#include <QList>
#include "common/kv.h"

class ToolTip
{
public:
	void insert(const QString &key, const QString &value);
	void setImage(const QString &img) {_img = img;}
	QString toString();

private:
	QList<KV> _list;
	QString _img;
};

#endif // TOOLTIP_H
