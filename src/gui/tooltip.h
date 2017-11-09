#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <QString>
#include <QList>
#include <QPair>

class ToolTip
{
public:
	void insert(const QString &key, const QString &value);
	QString toString();

private:
	QList<QPair<QString, QString> > _list;
};

#endif // TOOLTIP_H
