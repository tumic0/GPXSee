#ifndef MAPLIST_H
#define MAPLIST_H

#include <QList>
#include "map.h"


class MapList
{
public:
	static QList<Map*> load(const QString &fileName);
};

#endif // MAPLIST_H
