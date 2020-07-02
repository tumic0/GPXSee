#include <QDir>
#include <QFileInfo>
#include <QEventLoop>
#include <QPixmapCache>
#include <QImageReader>
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QtCore>
#else // QT_VERSION < 5
#include <QtConcurrent>
#endif // QT_VERSION < 5
#include "tileloader.h"


class TileImage
{
public:
	TileImage() : _tile(0), _scaledSize(0) {}
	TileImage(const QString &file, Tile *tile, int scaledSize)
	  : _file(file), _tile(tile), _scaledSize(scaledSize) {}

	void createPixmap()
	{
		_tile->pixmap().convertFromImage(_image);
	}
	void load()
	{
		QByteArray z(_tile->zoom().toString().toLatin1());
		QImageReader reader(_file, z);
		if (_scaledSize)
			reader.setScaledSize(QSize(_scaledSize, _scaledSize));
		reader.read(&_image);
	}

	const QString &file() const {return _file;}
	Tile *tile() {return _tile;}

private:
	QString _file;
	Tile *_tile;
	int _scaledSize;
	QImage _image;
};

static QString quadKey(const QPoint &xy, int zoom)
{
	QString qk;

	for (int i = zoom; i > 0; i--) {
		char digit = '0';
		unsigned mask = 1 << (i - 1);
		if (xy.x() & mask)
			digit++;
		if (xy.y() & mask)
			digit += 2;
		qk.append(digit);
	}

	return qk;
}

TileLoader::TileLoader(const QString &dir, QObject *parent)
  : QObject(parent), _dir(dir), _scaledSize(0), _quadTiles(false)
{
	if (!QDir().mkpath(_dir))
		qWarning("%s: %s", qPrintable(_dir), "Error creating tiles directory");

	_downloader = new Downloader(this);
	connect(_downloader, SIGNAL(finished()), this, SIGNAL(finished()));
}

void TileLoader::loadTilesAsync(QVector<Tile> &list)
{
	QList<Download> dl;
	QList<TileImage> imgs;

	for (int i = 0; i < list.size(); i++) {
		Tile &t = list[i];
		QString file(tileFile(t));

		if (QPixmapCache::find(file, t.pixmap()))
			continue;

		QFileInfo fi(file);

		if (fi.exists())
			imgs.append(TileImage(file, &t, _scaledSize));
		else {
			QUrl url(tileUrl(t));
			if (url.isLocalFile())
				imgs.append(TileImage(url.toLocalFile(), &t, _scaledSize));
			else
				dl.append(Download(url, file));
		}
	}

	if (!dl.empty())
		_downloader->get(dl, _authorization);

	QFuture<void> future = QtConcurrent::map(imgs, &TileImage::load);
	future.waitForFinished();

	for (int i = 0; i < imgs.size(); i++) {
		TileImage &ti = imgs[i];
		ti.createPixmap();
		QPixmapCache::insert(ti.file(), ti.tile()->pixmap());
	}
}

void TileLoader::loadTilesSync(QVector<Tile> &list)
{
	QList<Download> dl;
	QList<Tile *> tl;
	QList<TileImage> imgs;

	for (int i = 0; i < list.size(); i++) {
		Tile &t = list[i];
		QString file(tileFile(t));

		if (QPixmapCache::find(file, t.pixmap()))
			continue;

		QFileInfo fi(file);

		if (fi.exists())
			imgs.append(TileImage(file, &t, _scaledSize));
		else {
			QUrl url(tileUrl(t));
			if (url.isLocalFile())
				imgs.append(TileImage(url.toLocalFile(), &t, _scaledSize));
			else {
				dl.append(Download(url, file));
				tl.append(&t);
			}
		}
	}

	if (!dl.empty()) {
		QEventLoop wait;
		QObject::connect(_downloader, SIGNAL(finished()), &wait, SLOT(quit()));
		if (_downloader->get(dl, _authorization))
			wait.exec();

		for (int i = 0; i < tl.size(); i++) {
			Tile *t = tl[i];
			QString file = tileFile(*t);
			if (QFileInfo(file).exists())
				imgs.append(TileImage(file, t, _scaledSize));
		}
	}

	QFuture<void> future = QtConcurrent::map(imgs, &TileImage::load);
	future.waitForFinished();

	for (int i = 0; i < imgs.size(); i++)
		imgs[i].createPixmap();
}

void TileLoader::clearCache()
{
	QDir dir = QDir(_dir);
	QStringList list = dir.entryList();

	for (int i = 0; i < list.count(); i++)
		dir.remove(list.at(i));

	_downloader->clearErrors();

	QPixmapCache::clear();
}

void TileLoader::setScaledSize(int size)
{
	if (_scaledSize == size)
		return;

	_scaledSize = size;
	QPixmapCache::clear();
}

QUrl TileLoader::tileUrl(const Tile &tile) const
{
	QString url(_url);

	if (!tile.bbox().isNull()) {
		QString bbox = QString("%1,%2,%3,%4").arg(
		  QString::number(tile.bbox().left(), 'f', 6),
		  QString::number(tile.bbox().bottom(), 'f', 6),
		  QString::number(tile.bbox().right(), 'f', 6),
		  QString::number(tile.bbox().top(), 'f', 6));
		url.replace("$bbox", bbox);
	} else if (_quadTiles) {
		url.replace("$quadkey", quadKey(tile.xy(), tile.zoom().toInt()));
	} else {
		url.replace("$z", tile.zoom().toString());
		url.replace("$x", QString::number(tile.xy().x()));
		url.replace("$y", QString::number(tile.xy().y()));
	}

	return QUrl(url);
}

QString TileLoader::tileFile(const Tile &tile) const
{
	return _dir + QLatin1Char('/') + tile.zoom().toString() + QLatin1Char('-')
	  + QString::number(tile.xy().x()) + QLatin1Char('-')
	  + QString::number(tile.xy().y());
}
