#include <QFileInfo>
#include <QDir>
#include "downloader.h"
#include "config.h"
#include "map.h"


Map::Map(const QString &name, const QString &url, Downloader *downloader,
  QObject *parent) : QObject(parent)
{
	_name = name;
	_url = url;

	_downloader = downloader;


	connect(_downloader, SIGNAL(finished()), this, SLOT(emitLoaded()));

	QString path = TILES_DIR + QString("/") + _name;
	if (!QDir().mkpath(path))
		fprintf(stderr, "Error creating tiles dir: %s\n", qPrintable(path));
}

void Map::emitLoaded()
{
	emit loaded();
}

void Map::loadTiles(QList<Tile> &list, bool block)
{
	if (block)
		loadTilesSync(list);
	else
		loadTilesAsync(list);
}

void Map::loadTilesAsync(QList<Tile> &list)
{
	QList<Download> dl;

	for (int i = 0; i < list.size(); i++) {
		Tile &t = list[i];
		QString file = tileFile(t);
		QFileInfo fi(file);

		if (!fi.exists()) {
			fillTile(t);
			dl.append(Download(tileUrl(t), file));
		} else
			loadTileFile(t, file);
	}

	if (!dl.empty())
		_downloader->get(dl);
}

void Map::loadTilesSync(QList<Tile> &list)
{
	QList<Download> dl;

	for (int i = 0; i < list.size(); i++) {
		Tile &t = list[i];
		QString file = tileFile(t);
		QFileInfo fi(file);

		if (!fi.exists())
			dl.append(Download(tileUrl(t), file));
		else
			loadTileFile(t, file);
	}

	if (dl.empty())
		return;

	QEventLoop wait;
	connect(_downloader, SIGNAL(finished()), &wait, SLOT(quit()));
	if (_downloader->get(dl))
		wait.exec();

	for (int i = 0; i < list.size(); i++) {
		Tile &t = list[i];

		if (t.pixmap().isNull()) {
			QString file = tileFile(t);
			QFileInfo fi(file);

			if (!(fi.exists() && loadTileFile(t, file)))
				fillTile(t);
		}
	}
}

void Map::fillTile(Tile &tile)
{
	tile.pixmap() = QPixmap(Tile::size(), Tile::size());
	tile.pixmap().fill();
}

bool Map::loadTileFile(Tile &tile, const QString &file)
{
	if (!tile.pixmap().load(file)) {
		fprintf(stderr, "%s: error loading tile file\n", qPrintable(file));
		return false;
	}

	return true;
}

QString Map::tileUrl(const Tile &tile)
{
	QString url(_url);

	url.replace("$z", QString::number(tile.zoom()));
	url.replace("$x", QString::number(tile.xy().x()));
	url.replace("$y", QString::number(tile.xy().y()));

	return url;
}

QString Map::tileFile(const Tile &tile)
{
	QString file = TILES_DIR + QString("/%1/%2-%3-%4").arg(_name)
	  .arg(tile.zoom()).arg(tile.xy().x()).arg(tile.xy().y());

	return file;
}

void Map::clearCache()
{
	QString path = TILES_DIR + QString("/") + _name;
	QDir dir = QDir(path);
	QStringList list = dir.entryList();

	for (int i = 0; i < list.count(); i++)
		dir.remove(list.at(i));
}
