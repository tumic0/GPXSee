#ifndef MAPLIST_H
#define MAPLIST_H

#include <QList>
#include <QString>
#include <QObject>

class Map;

class MapList
{
public:
	static QList<Map*> load(QObject *parent = 0,
	  const QString &fileName = QString());
};

#endif // MAPLIST_H
