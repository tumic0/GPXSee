#ifndef TILELOADER_H
#define TILELOADER_H

#include <QObject>
#include <QString>
#include "tile.h"
#include "downloader.h"

class TileLoader : public QObject
{
	Q_OBJECT

public:
	TileLoader(QObject *parent = 0);

	void setUrl(const QString &url) {_url = url;}
	void setDir(const QString &dir) {_dir = dir;}
	void setAuthorization(const Authorization &authorization)
	  {_authorization = authorization;}

	void loadTilesAsync(QList<Tile> &list);
	void loadTilesSync(QList<Tile> &list);
	void clearCache();

signals:
	void finished();

private:
	QString tileUrl(const Tile &tile) const;
	QString tileFile(const Tile &tile) const;

	Downloader *_downloader;
	QString _url;
	QString _dir;
	Authorization _authorization;
};

#endif // TILELOADER_Honlinemap
