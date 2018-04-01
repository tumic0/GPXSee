#ifndef TILELOADER_H
#define TILELOADER_H

#include <QString>
#include "tile.h"
#include "downloader.h"

class TileLoader
{
public:
	TileLoader() {}
	TileLoader(const QString &url, const QString &dir,
	  const Authorization &authorization = Authorization())
	  : _url(url), _dir(dir), _authorization(authorization) {}

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
	Authorization _authorization;

	static Downloader *_downloader;
};

#endif // TILELOADER_Honlinemap
