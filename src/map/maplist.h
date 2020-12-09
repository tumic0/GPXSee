#ifndef MAPLIST_H
#define MAPLIST_H

#include <QString>

class Map;

class MapList
{
public:
	static QList<Map*> loadMaps(const QString &path);
	static QString formats();
	static QStringList filter();

private:
	static Map *loadFile(const QString &path, bool *terminate);
	static QList<Map*> loadDir(const QString &path);
};

#endif // MAPLIST_H
