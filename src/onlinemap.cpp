#include <QFileInfo>
#include <QDir>
#include <QPainter>
#include "downloader.h"
#include "config.h"
#include "rd.h"
#include "wgs84.h"
#include "misc.h"
#include "coordinates.h"
#include "mercator.h"
#include "onlinemap.h"


#define ZOOM_MAX      18
#define ZOOM_MIN      3
#define TILE_SIZE     256

static QPoint mercator2tile(const QPointF &m, int z)
{
	QPoint tile;

	tile.setX((int)(floor((m.x() + 180.0) / 360.0 * (1<<z))));
	tile.setY((int)(floor((1.0 - (m.y() / 180.0)) / 2.0 * (1<<z))));

	return tile;
}

static int scale2zoom(qreal scale)
{
	int zoom = (int)log2(360.0/(scale * (qreal)TILE_SIZE));

	if (zoom < ZOOM_MIN)
		return ZOOM_MIN;
	if (zoom > ZOOM_MAX)
		return ZOOM_MAX;

	return zoom;
}


Downloader *OnlineMap::downloader;

OnlineMap::OnlineMap(const QString &name, const QString &url, QObject *parent)
  : Map(parent)
{
	_name = name;
	_url = url;
	_block = false;
	_scale = ((360.0/(qreal)(1<<ZOOM_MAX))/(qreal)TILE_SIZE);

	connect(downloader, SIGNAL(finished()), this, SLOT(emitLoaded()));

	QString path = TILES_DIR + QString("/") + name;
	if (!QDir().mkpath(path))
		qWarning("Error creating tiles dir: %s\n", qPrintable(path));
}

void OnlineMap::emitLoaded()
{
	emit loaded();
}

void OnlineMap::loadTilesAsync(QList<Tile> &list)
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
		downloader->get(dl);
}

void OnlineMap::loadTilesSync(QList<Tile> &list)
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
	connect(downloader, SIGNAL(finished()), &wait, SLOT(quit()));
	if (downloader->get(dl))
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

void OnlineMap::fillTile(Tile &tile)
{
	tile.pixmap() = QPixmap(TILE_SIZE, TILE_SIZE);
	tile.pixmap().fill();
}

bool OnlineMap::loadTileFile(Tile &tile, const QString &file)
{
	if (!tile.pixmap().load(file)) {
		qWarning("%s: error loading tile file\n", qPrintable(file));
		return false;
	}

	return true;
}

QString OnlineMap::tileUrl(const Tile &tile)
{
	QString url(_url);

	url.replace("$z", QString::number(tile.zoom()));
	url.replace("$x", QString::number(tile.xy().x()));
	url.replace("$y", QString::number(tile.xy().y()));

	return url;
}

QString OnlineMap::tileFile(const Tile &tile)
{
	QString file = TILES_DIR + QString("/%1/%2-%3-%4").arg(name())
	  .arg(tile.zoom()).arg(tile.xy().x()).arg(tile.xy().y());

	return file;
}

void OnlineMap::clearCache()
{
	QString path = TILES_DIR + QString("/") + name();
	QDir dir = QDir(path);
	QStringList list = dir.entryList();

	for (int i = 0; i < list.count(); i++)
		dir.remove(list.at(i));
}

QRectF OnlineMap::bounds() const
{
	return scaled(QRectF(QPointF(-180, -180), QSizeF(360, 360)), 1.0/_scale);
}

qreal OnlineMap::zoomFit(const QSize &size, const QRectF &br)
{
	if (br.isNull())
		_scale = ((360.0/(qreal)(1<<ZOOM_MAX))/(qreal)TILE_SIZE);
	else {
		Coordinates topLeft(br.topLeft());
		Coordinates bottomRight(br.bottomRight());
		QRectF tbr(Mercator().ll2xy(topLeft), Mercator().ll2xy(bottomRight));

		QPointF sc(tbr.width() / size.width(), tbr.height() / size.height());

		_scale = ((360.0/(qreal)(1<<scale2zoom(qMax(sc.x(), sc.y()))))
		  / (qreal)TILE_SIZE);
	}

	return _scale;
}

qreal OnlineMap::resolution(const QPointF &p) const
{
	return (WGS84_RADIUS * 2 * M_PI * _scale / 360.0
	  * cos(2.0 * atan(exp(deg2rad(-p.y() * _scale))) - M_PI/2));
}

qreal OnlineMap::zoomIn()
{
	int zoom = qMin(scale2zoom(_scale) + 1, ZOOM_MAX);
	_scale = ((360.0/(qreal)(1<<zoom))/(qreal)TILE_SIZE);
	return _scale;
}

qreal OnlineMap::zoomOut()
{
	int zoom = qMax(scale2zoom(_scale) - 1, ZOOM_MIN);
	_scale = ((360.0/(qreal)(1<<zoom))/(qreal)TILE_SIZE);
	return _scale;
}

void OnlineMap::draw(QPainter *painter, const QRectF &rect)
{
	int zoom = scale2zoom(_scale);

	QPoint tile = mercator2tile(QPointF(rect.topLeft().x() * _scale,
	  -rect.topLeft().y() * _scale), zoom);
	QPoint tl = QPoint((int)floor(rect.left() / (qreal)TILE_SIZE)
	  * TILE_SIZE, (int)floor(rect.top() / TILE_SIZE) * TILE_SIZE);

	QList<Tile> tiles;
	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	for (int i = 0; i < ceil(s.width() / TILE_SIZE); i++)
		for (int j = 0; j < ceil(s.height() / TILE_SIZE); j++)
			tiles.append(Tile(QPoint(tile.x() + i, tile.y() + j), zoom));

	if (_block)
		loadTilesSync(tiles);
	else
		loadTilesAsync(tiles);

	for (int i = 0; i < tiles.count(); i++) {
		Tile &t = tiles[i];
		QPoint tp(tl.x() + (t.xy().x() - tile.x()) * TILE_SIZE,
		  tl.y() + (t.xy().y() - tile.y()) * TILE_SIZE);
		painter->drawPixmap(tp, t.pixmap());
	}
}

QPointF OnlineMap::ll2xy(const Coordinates &c) const
{
	QPointF m = Mercator().ll2xy(c);
	return QPointF(m.x() / _scale, m.y() / -_scale);
}

Coordinates OnlineMap::xy2ll(const QPointF &p) const
{
	QPointF m(p.x() * _scale, -p.y() * _scale);
	return Mercator().xy2ll(m);
}
