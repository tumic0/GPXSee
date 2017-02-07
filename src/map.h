#ifndef MAP_H
#define MAP_H

#include "tile.h"

class Downloader;

class Map : public QObject
{
	Q_OBJECT

public:
	Map(const QString &name, const QString &url, Downloader *downloader,
	  QObject *parent = 0);

	const QString &name() const {return _name;}
	void loadTiles(QList<Tile> &list, bool block);
	void clearCache();

signals:
	void loaded();

private slots:
	void emitLoaded();

private:
	QString tileUrl(const Tile &tile);
	QString tileFile(const Tile &tile);
	bool loadTileFile(Tile &tile, const QString &file);
	void fillTile(Tile &tile);

	void loadTilesAsync(QList<Tile> &list);
	void loadTilesSync(QList<Tile> &list);

	Downloader *_downloader;

	QString _name;
	QString _url;
};

#endif // MAP_H
