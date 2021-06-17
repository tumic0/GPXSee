#ifndef MAPLIST_H
#define MAPLIST_H

#include <QString>
#include "common/treenode.h"

class Map;
class Projection;

class MapList
{
public:
	static TreeNode<Map*> loadMaps(const QString &path, const Projection &proj);
	static QString formats();
	static QStringList filter();

private:
	static Map *loadFile(const QString &path, const Projection &proj,
	  bool *isDir = 0);
	static TreeNode<Map*> loadDir(const QString &path, const Projection &proj,
	  TreeNode<Map *> *parent = 0);
};

#endif // MAPLIST_H
