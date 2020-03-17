#ifndef MAPLIST_H
#define MAPLIST_H

#include <QString>

class Map;

class MapList
{
public:
	static QList<Map*> loadMaps(const QString &path, QString &errorString);
	static QString formats();
	static QStringList filter();

private:
	static Map *loadFile(const QString &path, QString &errorString,
	  bool *terminate);
	static QList<Map*> loadDir(const QString &path, QString &errorString);
};

#endif // MAPLIST_H
