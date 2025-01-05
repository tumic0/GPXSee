#include <QtMath>
#include <QFileInfo>
#include "common/rectc.h"
#include "demloader.h"


static QList<DEM::Tile> tiles(const RectC &rect)
{
	QList<DEM::Tile> list;

	if (rect.isNull())
		return list;

	for (int i = qFloor(rect.top()); i >= qFloor(rect.bottom()); i--)
		for (int j = qFloor(rect.left()); j <= qFloor(rect.right()); j++)
			list.append(DEM::Tile(j, i));

	return list;
}

static bool isZip(const QUrl &url)
{
	QFileInfo fi(url.fileName());
	return (fi.suffix().toLower() == "zip");
}


DEMLoader::DEMLoader(const QString &dir, QObject *parent)
  : QObject(parent), _dir(dir)
{
	_downloader = new Downloader(this);
	connect(_downloader, &Downloader::finished, this, &DEMLoader::finished);
}

int DEMLoader::numTiles(const RectC &rect) const
{
	QList<DEM::Tile> tl(tiles(rect));
	int cnt = 0;

	for (int i = 0; i < tl.size(); i++) {
		const DEM::Tile &t = tl.at(i);
		QString fn(tileFile(t));
		QString zn(fn + ".zip");

		if (!(QFileInfo::exists(zn) || QFileInfo::exists(fn)))
			cnt++;
	}

	return cnt;
}

bool DEMLoader::loadTiles(const RectC &rect)
{
	QList<DEM::Tile> tl(tiles(rect));
	QList<Download> dl;

	/* Create the user DEM dir only when a download is requested as it will
	   override the global DEM dir. */
	if (!_dir.mkpath(_dir.absolutePath())) {
		qWarning("%s: %s", qUtf8Printable(_dir.canonicalPath()),
		  "Error creating DEM directory");
		return false;
	}
	DEM::setDir(_dir.path());

	for (int i = 0; i < tl.size(); i++) {
		const DEM::Tile &t = tl.at(i);
		QString fn(tileFile(t));
		QString zn(fn + ".zip");

		if (!(QFileInfo::exists(zn) || QFileInfo::exists(fn))) {
			QUrl url(tileUrl(t));
			dl.append(Download(url, isZip(url) ? zn : fn));
		}

		if (dl.size() > DEM_DOWNLOAD_LIMIT) {
			qWarning("DEM download limit (%d) exceeded.", DEM_DOWNLOAD_LIMIT);
			return false;
		}
	}

	return _downloader->get(dl, _headers);
}

bool DEMLoader::checkTiles(const RectC &rect) const
{
	QList<DEM::Tile> tl(tiles(rect));

	for (int i = 0; i < tl.size(); i++) {
		const DEM::Tile &t = tl.at(i);
		QString fn(tileFile(t));
		QString zn(fn + ".zip");

		if (!(QFileInfo::exists(zn) || QFileInfo::exists(fn)))
			return false;
	}

	return true;
}

QUrl DEMLoader::tileUrl(const DEM::Tile &tile) const
{
	QString url(_url);

	url.replace("$lon", tile.lonStr());
	url.replace("$lat", tile.latStr());

	return QUrl(url);
}

QString DEMLoader::tileFile(const DEM::Tile &tile) const
{
	return _dir.absoluteFilePath(tile.fileName());
}

void DEMLoader::setAuthorization(const Authorization &authorization)
{
	QList<HTTPHeader> headers;
	if (!authorization.isNull())
		headers.append(authorization.header());
	_headers = headers;
}
