#include <QDir>
#include <QFileInfo>
#include <QEventLoop>
#include "downloader.h"
#include "tileloader.h"


static bool loadTileFile(Tile &tile, const QString &file)
{
	if (!tile.pixmap().load(file)) {
		qWarning("%s: error loading tile file\n", qPrintable(file));
		return false;
	}

	return true;
}

Downloader *TileLoader::_downloader = 0;

void TileLoader::loadTilesAsync(QList<Tile> &list)
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

	if (!dl.empty())
		_downloader->get(dl, _authorization);
}

void TileLoader::loadTilesSync(QList<Tile> &list)
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
	QObject::connect(_downloader, SIGNAL(finished()), &wait, SLOT(quit()));
	if (_downloader->get(dl, _authorization))
		wait.exec();

	for (int i = 0; i < list.size(); i++) {
		Tile &t = list[i];

		if (t.pixmap().isNull()) {
			QString file = tileFile(t);
			if (QFileInfo(file).exists())
				loadTileFile(t, file);
		}
	}
}

void TileLoader::clearCache()
{
	QDir dir = QDir(_dir);
	QStringList list = dir.entryList();

	for (int i = 0; i < list.count(); i++)
		dir.remove(list.at(i));

	_downloader->clearErrors();
}

QString TileLoader::tileUrl(const Tile &tile) const
{
	QString url(_url);

	if (!tile.bbox().isNull()) {
		QString bbox = QString("%1,%2,%3,%4").arg(
		  QString::number(tile.bbox().left(), 'f', 6),
		  QString::number(tile.bbox().bottom(), 'f', 6),
		  QString::number(tile.bbox().right(), 'f', 6),
		  QString::number(tile.bbox().top(), 'f', 6));
		url.replace("$bbox", bbox);
	} else {
		url.replace("$z", tile.zoom().toString());
		url.replace("$x", QString::number(tile.xy().x()));
		url.replace("$y", QString::number(tile.xy().y()));
	}

	return url;
}

QString TileLoader::tileFile(const Tile &tile) const
{
	QString file = _dir + QString("/%1-%2-%3").arg(tile.zoom().toString())
	  .arg(tile.xy().x()).arg(tile.xy().y());

	return file;
}
