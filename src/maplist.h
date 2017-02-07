#ifndef MAPLIST_H
#define MAPLIST_H

#include <QList>
#include <QString>
#include <QObject>

class Map;
class Downloader;

class MapList
{
public:
	MapList(Downloader *downloader) : _downloader(downloader) {}
	QList<Map*> load(const QString &fileName, QObject *parent = 0);

private:
	Downloader *_downloader;
};

#endif // MAPLIST_H
