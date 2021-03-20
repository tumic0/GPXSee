#ifndef MAPLIST_H
#define MAPLIST_H

#include <QString>
#include "common/treenode.h"

class Map;

class MapList
{
public:
	static TreeNode<Map*> loadMaps(const QString &path);
	static QString formats();
	static QStringList filter();

private:
	static Map *loadFile(const QString &path, bool *isDir = 0);
	static TreeNode<Map*> loadDir(const QString &path,
	  TreeNode<Map *> *parent = 0);
};

#endif // MAPLIST_H
