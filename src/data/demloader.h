#ifndef DEMLOADER_H
#define DEMLOADER_H

#include <QObject>
#include <QDir>
#include "common/downloader.h"
#include "common/dem.h"

class RectC;

class DEMLoader : public QObject
{
	Q_OBJECT

public:
	DEMLoader(const QString &dir, QObject *parent = 0);

	void setUrl(const QString &url) {_url = url;}
	void setAuthorization(const Authorization &authorization);

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
