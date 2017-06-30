#include <QFileInfo>
#include <QDir>
#include <QPainter>
#include "rectc.h"
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

static qreal zoom2scale(int zoom)
{
	return (360.0/(qreal)((1<<zoom) * TILE_SIZE));
}

static int scale2zoom(qreal scale)
{
	return (int)log2(360.0/(scale * (qreal)TILE_SIZE));
}


Downloader *OnlineMap::downloader;

OnlineMap::OnlineMap(const QString &name, const QString &url, QObject *parent)
  : Map(parent)
{
	_name = name;
	_url = url;
	_block = false;
	_zoom = ZOOM_MAX;

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
	return scaled(QRectF(QPointF(-180, -180), QSizeF(360, 360)),
	  1.0/zoom2scale(_zoom));
}

qreal OnlineMap::zoomFit(const QSize &size, const RectC &br)
{
	if (!br.isValid())
		_zoom = ZOOM_MAX;
	else {
		QRectF tbr(Mercator().ll2xy(br.topLeft()),
		  Mercator().ll2xy(br.bottomRight()));
		QPointF sc(tbr.width() / size.width(), tbr.height() / size.height());

		_zoom = scale2zoom(qMax(sc.x(), sc.y()));
		if (_zoom < ZOOM_MIN)
			_zoom = ZOOM_MIN;
		if (_zoom > ZOOM_MAX)
			_zoom = ZOOM_MAX;
	}

	return _zoom;
}

qreal OnlineMap::zoomFit(qreal resolution, const Coordinates &c)
{
	_zoom = (int)(log2((WGS84_RADIUS * 2 * M_PI * cos(deg2rad(c.lat())))
	  / resolution) - log2(TILE_SIZE));

	if (_zoom < ZOOM_MIN)
		_zoom = ZOOM_MIN;
	if (_zoom > ZOOM_MAX)
		_zoom = ZOOM_MAX;

	return _zoom;
}

qreal OnlineMap::resolution(const QPointF &p) const
{
	qreal scale = zoom2scale(_zoom);

	return (WGS84_RADIUS * 2 * M_PI * scale / 360.0
	  * cos(2.0 * atan(exp(deg2rad(-p.y() * scale))) - M_PI/2));
}

qreal OnlineMap::zoomIn()
{
	_zoom = qMin(_zoom + 1, ZOOM_MAX);
	return _zoom;
}

qreal OnlineMap::zoomOut()
{
	_zoom = qMax(_zoom - 1, ZOOM_MIN);
	return _zoom;
}

void OnlineMap::draw(QPainter *painter, const QRectF &rect)
{
	qreal scale = zoom2scale(_zoom);

	QPoint tile = mercator2tile(QPointF(rect.topLeft().x() * scale,
	  -rect.topLeft().y() * scale), _zoom);
	QPoint tl = QPoint((int)floor(rect.left() / (qreal)TILE_SIZE)
	  * TILE_SIZE, (int)floor(rect.top() / TILE_SIZE) * TILE_SIZE);

	QList<Tile> tiles;
	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	for (int i = 0; i < ceil(s.width() / TILE_SIZE); i++)
		for (int j = 0; j < ceil(s.height() / TILE_SIZE); j++)
			tiles.append(Tile(QPoint(tile.x() + i, tile.y() + j), _zoom));

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

QPointF OnlineMap::ll2xy(const Coordinates &c)
{
	qreal scale = zoom2scale(_zoom);
	QPointF m = Mercator().ll2xy(c);
	return QPointF(m.x() / scale, m.y() / -scale);
}

Coordinates OnlineMap::xy2ll(const QPointF &p)
{
	qreal scale = zoom2scale(_zoom);
	QPointF m(p.x() * scale, -p.y() * scale);
	return Mercator().xy2ll(m);
}
