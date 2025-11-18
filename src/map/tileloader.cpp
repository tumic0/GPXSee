#include <QFileInfo>
#include <QEventLoop>
#include "tileloader.h"

#define SUBSTITUTE_CHAR '$'

static bool inline IS_INT(const QVariant &v)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	return (static_cast<QMetaType::Type>(v.type()) == QMetaType::Int);
#else // QT 6
	return (static_cast<QMetaType::Type>((v.typeId()) == QMetaType::Int));
#endif // QT 6
}

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
	if (!_dir.mkpath("."))
		qWarning("%s: %s", qUtf8Printable(_dir.absolutePath()),
		  "Error creating tiles directory");

	_downloader = new Downloader(this);
	connect(_downloader, &Downloader::finished, this, &TileLoader::finished);
}

void TileLoader::loadTilesAsync(QVector<Tile> &list)
{
	QList<Download> dl;

	for (int i = 0; i < list.size(); i++) {
		Tile &t = list[i];

		for (int j = 0; j < _url.size(); j++) {
			QString file(tileFile(t, j));

			if (QFileInfo::exists(file))
				t.addFile(file);
			else {
				QUrl url(tileUrl(t, j));
				if (url.isLocalFile())
					t.addFile(url.toLocalFile());
				else {
					t.addFile(QString());
					dl.append(Download(url, file));
				}
			}
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

		for (int j = 0; j < _url.size(); j++) {
			QString file(tileFile(t, j));

			if (QFileInfo::exists(file))
				t.addFile(file);
			else {
				QUrl url(tileUrl(t, j));
				if (url.isLocalFile())
					t.addFile(url.toLocalFile());
				else {
					t.addFile(QString());
					dl.append(Download(url, file));
				}
			}
		}

		for (int j = 0; j < _url.size(); j++) {
			if (t.files().at(j).isNull()) {
				tl.append(&t);
				break;
			}
		}
	}

	if (!dl.empty()) {
		QEventLoop wait;
		connect(_downloader, &Downloader::finished, &wait, &QEventLoop::quit);
		if (_downloader->get(dl, _headers))
			wait.exec();

		for (int i = 0; i < tl.size(); i++) {
			for (int j = 0; j < _url.size(); j++) {
				Tile *t = tl[i];
				if (t->files().at(j).isNull()) {
					QString file(tileFile(*t, j));
					if (QFileInfo::exists(file))
						t->setFile(j, file);
				}
			}
		}
	}
}

void TileLoader::clearCache()
{
	QStringList list = _dir.entryList();

	for (int i = 0; i < list.count(); i++)
		_dir.remove(list.at(i));

	_downloader->clearErrors();
}

QUrl TileLoader::tileUrl(const Tile &tile, int layer) const
{
	QString url(_url.at(layer));

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

QString TileLoader::tileFile(const Tile &tile, int layer) const
{
	QString zoom(IS_INT(tile.zoom())
	  ? tile.zoom().toString() : fsSafeStr(tile.zoom().toString()));
	QString ls(layer ? "#" + QString::number(layer) : QString());

	return _dir.absoluteFilePath(zoom + QLatin1Char('-')
	  + QString::number(tile.xy().x()) + QLatin1Char('-')
	  + QString::number(tile.xy().y()) + ls);
}
