#ifndef TILELOADER_H
#define TILELOADER_H

#include <QObject>
#include <QString>
#include "common/downloader.h"
#include "tile.h"

class TileLoader : public QObject
{
	Q_OBJECT

public:
	TileLoader(const QString &dir, QObject *parent = 0);

	void setUrl(const QString &url) {_url = url;}
	void setAuthorization(const Authorization &authorization)
	  {_authorization = authorization;}
	void setScaledSize(int size);
	void setQuadTiles(bool quadTiles) {_quadTiles = quadTiles;}

	void loadTilesAsync(QVector<FetchTile> &list);
	void loadTilesSync(QVector<FetchTile> &list);
	void clearCache();

signals:
	void finished();

private:
	QUrl tileUrl(const FetchTile &tile) const;
	QString tileFile(const FetchTile &tile) const;

	Downloader *_downloader;
	QString _url;
	QString _dir;
	Authorization _authorization;
	int _scaledSize;
	bool _quadTiles;
};

#endif // TILELOADER_H
