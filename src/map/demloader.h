#ifndef DEMLOADER_H
#define DEMLOADER_H

#include <QObject>
#include <QDir>
#include "downloader.h"
#include "dem.h"

class RectC;

#define DEM_DOWNLOAD_WARNING 4
#define DEM_DOWNLOAD_LIMIT   1024

class DEMLoader : public QObject
{
	Q_OBJECT

public:
	DEMLoader(const QString &dir, QObject *parent = 0);

	void setUrl(const QString &url) {_url = url;}
	void setAuthorization(const Authorization &authorization);

	int numTiles(const RectC &rect) const;
	bool loadTiles(const RectC &rect);
	bool checkTiles(const RectC &rect) const;

	const QString &url() const {return _url;}

signals:
	void finished();

private:
	QUrl tileUrl(const DEM::Tile &tile) const;
	QString tileFile(const DEM::Tile &tile) const;

	Downloader *_downloader;
	QString _url;
	QDir _dir;
	QList<HTTPHeader> _headers;
};

#endif // DEMLOADER_H
