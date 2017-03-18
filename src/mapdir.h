#ifndef MAPDIR_H
#define MAPDIR_H

#include <QList>
#include <QString>

class QObject;
class Map;

class MapDir
{
public:
	static QList<Map*> load(const QString &path, QObject *parent = 0);
};

#endif // MAPDIR_H
