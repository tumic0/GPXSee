#include <QDir>
#include <QFileInfo>
#include <QEventLoop>
#include "tileloader.h"

#define SUBSTITUTE_CHAR '$'
#define IS_INT(zoom) \
	((QMetaType::Type)((zoom).type()) == QMetaType::Int)


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
  : QObject(parent), _urlType(XYZ), _dir(dir)
{
	if (!QDir().mkpath(_dir))
		qWarning("%s: %s", qPrintable(_dir), "Error creating tiles directory");

	_downloader = new Downloader(this);
	connect(_downloader, &Downloader::finished, this, &TileLoader::finished);
}


void TileLoader::loadTilesAsync(QVector<Tile> &list)
{
	QList<Download> dl;

	for (int i = 0; i < list.size(); i++) {
		Tile &t = list[i];
		QString file(tileFile(t));

		if (QFileInfo::exists(file))
			t.setFile(file);
		else {
			QUrl url(tileUrl(t));
			if (url.isLocalFile())
				t.setFile(url.toLocalFile());
			else
				dl.append(Download(url, file));
		}
	}

	if (!dl.empty())
		_downloader->get(dl, _headers);
}

void TileLoader::loadTilesSync(QVector<Tile> &list)
{
	QList<Download> dl;
	QList<Tile *> tl;

	for (int i = 0; i < list.size(); i++) {
		Tile &t = list[i];
		QString file(tileFile(t));

		if (QFileInfo::exists(file))
			t.setFile(file);
		else {
			QUrl url(tileUrl(t));
			if (url.isLocalFile())
				t.setFile(url.toLocalFile());
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
			Tile *t = tl[i];
			QString file = tileFile(*t);
			if (QFileInfo(file).exists())
				t->setFile(file);
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

QUrl TileLoader::tileUrl(const Tile &tile) const
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

QString TileLoader::tileFile(const Tile &tile) const
{
	QString zoom(IS_INT(tile.zoom())
	  ? tile.zoom().toString() : fsSafeStr(tile.zoom().toString()));

	return _dir + QLatin1Char('/') + zoom + QLatin1Char('-')
	  + QString::number(tile.xy().x()) + QLatin1Char('-')
	  + QString::number(tile.xy().y());
}
