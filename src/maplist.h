#ifndef MAPLIST_H
#define MAPLIST_H

#include <QList>
#include <QString>

class QObject;
class Map;

class MapList
{
public:
	static QList<Map*> load(const QString &fileName, QObject *parent = 0);
};

#endif // MAPLIST_H
