#include <QDir>
#include <QFileInfo>
#include <QEventLoop>
#include <QPixmapCache>
#include <QImageReader>
#include <QtConcurrent>
#include "tileloader.h"

#define SUBSTITUTE_CHAR '$'

class TileImage
{
public:
	TileImage() : _tile(0), _scaledSize(0) {}
	TileImage(const QString &file, FetchTile *tile, int scaledSize)
	  : _file(file), _tile(tile), _scaledSize(scaledSize) {}

	void load()
	{
		QImage img;
		QByteArray z(_tile->zoom().toString().toLatin1());
		QImageReader reader(_file, z);
		if (_scaledSize)
			reader.setScaledSize(QSize(_scaledSize, _scaledSize));
		reader.read(&img);
		_tile->pixmap().convertFromImage(img);
	}

	const QString &file() const {return _file;}
	FetchTile *tile() {return _tile;}

private:
	QString _file;
	FetchTile *_tile;
	int _scaledSize;
};

static QString fsSafeStr(const QString &str)
{
	QString ret(str);

#ifdef Q_OS_WIN32
	ret.replace('<', SUBSTITUTE_CHAR);
	ret.replace('>', SUBSTITUTE_CHAR);
	ret.replace(':', SUBSTITUTE_CHAR);
	ret.replace('"', SUBSTITUTE_CHAR);
	ret.replace('/', SUBSTITUTE_CHAR);
	ret.replace('\\', SUBSTITUTE_CHAR);
	ret.replace('|', SUBSTITUTE_CHAR);
	ret.replace('?', SUBSTITUTE_CHAR);
	ret.replace('*', SUBSTITUTE_CHAR);
#else // Q_OS_WIN32
	ret.replace('/', SUBSTITUTE_CHAR);
#endif // Q_OS_WIN32

	return ret;
}

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
  : QObject(parent), _urlType(XYZ), _dir(dir), _scaledSize(0)
{
	if (!QDir().mkpath(_dir))
		qWarning("%s: %s", qPrintable(_dir), "Error creating tiles directory");

	_downloader = new Downloader(this);
	connect(_downloader, &Downloader::finished, this, &TileLoader::finished);
}


void TileLoader::loadTilesAsync(QVector<FetchTile> &list)
{
	QList<Download> dl;
	QList<TileImage> imgs;

	for (int i = 0; i < list.size(); i++) {
		FetchTile &t = list[i];
		QString file(tileFile(t));

		if (QPixmapCache::find(file, &t.pixmap()))
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
		_downloader->get(dl, _headers);

	QFuture<void> future = QtConcurrent::map(imgs, &TileImage::load);
	future.waitForFinished();

	for (int i = 0; i < imgs.size(); i++) {
		TileImage &ti = imgs[i];
		QPixmapCache::insert(ti.file(), ti.tile()->pixmap());
	}
}

void TileLoader::loadTilesSync(QVector<FetchTile> &list)
{
	QList<Download> dl;
	QList<FetchTile *> tl;
	QList<TileImage> imgs;

	for (int i = 0; i < list.size(); i++) {
		FetchTile &t = list[i];
		QString file(tileFile(t));

		if (QPixmapCache::find(file, &t.pixmap()))
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
		connect(_downloader, &Downloader::finished, &wait, &QEventLoop::quit);
		if (_downloader->get(dl, _headers))
			wait.exec();

		for (int i = 0; i < tl.size(); i++) {
			FetchTile *t = tl[i];
			QString file = tileFile(*t);
			if (QFileInfo(file).exists())
				imgs.append(TileImage(file, t, _scaledSize));
		}
	}

	QFuture<void> future = QtConcurrent::map(imgs, &TileImage::load);
	future.waitForFinished();

	for (int i = 0; i < imgs.size(); i++) {
		TileImage &ti = imgs[i];
		QPixmapCache::insert(ti.file(), ti.tile()->pixmap());
	}
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

QUrl TileLoader::tileUrl(const FetchTile &tile) const
{
	QString url(_url);

	switch (_urlType) {
		case BoundingBox:
			url.replace("$bbox", QString("%1,%2,%3,%4").arg(
			  QString::number(tile.bbox().left(), 'f', 6),
			  QString::number(tile.bbox().bottom(), 'f', 6),
			  QString::number(tile.bbox().right(), 'f', 6),
			  QString::number(tile.bbox().top(), 'f', 6)));
			break;
		case QuadTiles:
			url.replace("$quadkey", quadKey(tile.xy(), tile.zoom().toInt()));
			break;
		default:
			url.replace("$z", tile.zoom().toString());
			url.replace("$x", QString::number(tile.xy().x()));
			url.replace("$y", QString::number(tile.xy().y()));
	}

	return QUrl(url);
}

QString TileLoader::tileFile(const FetchTile &tile) const
{
	QString zoom(((QMetaType::Type)(tile.zoom().type()) == QMetaType::Int)
	  ? tile.zoom().toString() : fsSafeStr(tile.zoom().toString()));

	return _dir + QLatin1Char('/') + zoom + QLatin1Char('-')
	  + QString::number(tile.xy().x()) + QLatin1Char('-')
	  + QString::number(tile.xy().y());
}
