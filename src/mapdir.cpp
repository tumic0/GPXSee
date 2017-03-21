#include <QDir>
#include "mapdir.h"

#include <QDebug>
#include "offlinemap.h"

QList<Map*> MapDir::load(const QString &path, QObject *parent)
{
	QList<Map*> maps;
	QDir dir(path);


	if (!dir.exists())
		return maps;

	if (!dir.isReadable()) {
		qWarning("Map directory not readable: %s\n", qPrintable(path));
		return maps;
	}

	QFileInfoList list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
	for (int i = 0; i < list.size(); i++) {
		QFileInfo fileInfo = list.at(i);
		OfflineMap *map = new OfflineMap(fileInfo.absoluteFilePath(), parent);
		if (map->isValid())
			maps.append(map);
		else
			delete map;
	}

	return maps;
}
