#ifndef TILELOADER_H
#define TILELOADER_H

#include <QString>
#include "tile.h"

class Downloader;

class TileLoader
{
public:
	TileLoader() {}
	TileLoader(const QString &url, const QString &dir)
	  : _url(url), _dir(dir) {}

	void loadTilesAsync(QList<Tile> &list);
	void loadTilesSync(QList<Tile> &list);
	void clearCache();

	static Downloader *downloader() {return _downloader;}
	static void setDownloader(Downloader *downloader)
	  {_downloader = downloader;}

private:
	QString tileUrl(const Tile &tile) const;
	QString tileFile(const Tile &tile) const;

	QString _url;
	QString _dir;

	static Downloader *_downloader;
};

#endif // TILELOADER_Honlinemap
