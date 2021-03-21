#ifndef POIACTION_H
#define POIACTION_H

#include <QAction>
#include "common/util.h"

class POIAction : public QAction
{
	Q_OBJECT

public:
	POIAction(const QString &path, QObject *parent = 0)
	  : QAction(Util::file2name(path), parent)
	{
		setMenuRole(QAction::NoRole);
		setCheckable(true);
		setData(path);
	}
};

#endif // POIACTION_H
