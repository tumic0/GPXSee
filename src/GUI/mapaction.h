#ifndef MAPACTION_H
#define MAPACTION_H

#include <QAction>
#include "map/map.h"

class MapAction : public QAction
{
	Q_OBJECT

public:
	MapAction(Map *map, QObject *parent = 0) : QAction(map->name(), parent)
	{
		map->setParent(this);
		setData(QVariant::fromValue(map));
		setEnabled(map->isReady());
		connect(map, SIGNAL(mapLoaded()), this, SLOT(mapLoaded()));
	}

signals:
	void loaded();

private slots:
	void mapLoaded()
	{
		Map *map = data().value<Map*>();
		setEnabled(map->isValid());
		emit loaded();
	}
};

#endif // MAPACTION_H
