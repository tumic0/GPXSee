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
	TileLoader(const QString &dir, QObject *parent = 0);

	void setUrl(const QString &url) {_url = url;}
	void setAuthorization(const Authorization &authorization)
	  {_authorization = authorization;}
	void setScaledSize(int size) {_scaledSize = size;}
	void setQuadTiles(bool quadTiles) {_quadTiles = quadTiles;}

	void loadTilesAsync(QVector<Tile> &list);
	void loadTilesSync(QVector<Tile> &list);
	void clearCache();

signals:
	void finished();

private:
	QUrl tileUrl(const Tile &tile) const;
	QString tileFile(const Tile &tile) const;

	Downloader *_downloader;
	QString _url;
	QString _dir;
	Authorization _authorization;
	int _scaledSize;
	bool _quadTiles;
};

#endif // TILELOADER_Honlinemap
