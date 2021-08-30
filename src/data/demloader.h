#ifndef DEMLOADER_H
#define DEMLOADER_H

#include <QObject>
#include "common/downloader.h"
#include "dem.h"

class RectC;

class DEMLoader : public QObject
{
	Q_OBJECT

public:
	DEMLoader(const QString &dir, QObject *parent = 0);

	void setUrl(const QString &url) {_url = url;}
	void setAuthorization(const Authorization &authorization)
	  {_authorization = authorization;}

	bool loadTiles(const RectC &rect);
	bool checkTiles(const RectC &rect) const;

signals:
	void finished();

private:
	QUrl tileUrl(const DEM::Tile &tile) const;
	QString tileFile(const DEM::Tile &tile) const;

	Downloader *_downloader;
	QString _url;
	QString _dir;
	Authorization _authorization;
};

#endif // DEMLOADER_H
