#include <QFileInfo>
#include <QDir>
#include "downloader.h"
#include "ll.h"
#include "config.h"
#include "map.h"


Map::Map(QObject *parent, const QString &name, const QString &url)
  : QObject(parent)
{
	_name = name;
	_url = url;

	connect(&Downloader::instance(), SIGNAL(finished()), this,
	  SLOT(emitLoaded()));

	QString path = QString(TILES_DIR"/") + _name;
	if (!QDir::home().mkpath(path))
		fprintf(stderr, "Error creating tiles dir: %s\n", qPrintable(path));
}

void Map::emitLoaded()
{
	emit loaded();
}

void Map::loadTiles(QList<Tile> &list)
{
	QList<Download> dl;

	for (int i = 0; i < list.size(); ++i) {
		Tile &t = list[i];
		QString file = QString("%1/" TILES_DIR "/%2/%3-%4-%5")
		  .arg(QDir::homePath()).arg(_name).arg(t.zoom()).arg(t.xy().rx())
		  .arg(t.xy().ry());
		QFileInfo fi(file);

		if (fi.exists()) {
			if (!t.pixmap().load(file))
				fprintf(stderr, "Error loading map tile: %s\n",
				  qPrintable(file));
		} else {
			t.pixmap() = QPixmap(TILE_SIZE, TILE_SIZE);
			t.pixmap().fill();

			QString url(_url);
			url.replace("$z", QString::number(t.zoom()));
			url.replace("$x", QString::number(t.xy().x()));
			url.replace("$y", QString::number(t.xy().y()));
			dl.append(Download(url, file));
		}
	}

	if (!dl.empty())
		Downloader::instance().get(dl);
}
