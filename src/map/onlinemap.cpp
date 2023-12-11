#include <QPainter>
#include <QDir>
#include <QPixmapCache>
#include "common/rectc.h"
#include "common/programpaths.h"
#include "common/downloader.h"
#include "osm.h"
#include "onlinemap.h"


#define MAX_OVERZOOM 3

OnlineMap::OnlineMap(const QString &fileName, const QString &name,
  const QString &url, const Range &zooms, const RectC &bounds, qreal tileRatio,
  const QList<HTTPHeader> &headers, int tileSize, bool scalable, bool invertY,
  bool quadTiles, QObject *parent)
    : Map(fileName, parent), _name(name), _zooms(zooms), _bounds(bounds),
	_zoom(_zooms.max()), _tileSize(tileSize), _base(0), _mapRatio(1.0),
	_tileRatio(tileRatio), _scalable(scalable), _scaledSize(0), _invertY(invertY)
{
	_tileLoader = new TileLoader(QDir(ProgramPaths::tilesDir()).filePath(_name),
	  this);
	_tileLoader->setUrl(url, quadTiles ? TileLoader::QuadTiles : TileLoader::XYZ);
	_tileLoader->setHeaders(headers);
	connect(_tileLoader, &TileLoader::finished, this, &OnlineMap::tilesLoaded);

	if (_scalable) {
		_base = _zooms.max();
		_zooms.setMax(qMin(_zooms.max() + MAX_OVERZOOM, OSM::ZOOMS.max()));
	}
}

QRectF OnlineMap::bounds()
{
	return QRectF(ll2xy(_bounds.topLeft()), ll2xy(_bounds.bottomRight()));
}

int OnlineMap::limitZoom(int zoom) const
{
	if (zoom < _zooms.min())
		return _zooms.min();
	if (zoom > _zooms.max())
		return _zooms.max();

	return zoom;
}

int OnlineMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (!rect.isValid())
		_zoom = _zooms.max();
	else {
		QRectF tbr(OSM::ll2m(rect.topLeft()), OSM::ll2m(rect.bottomRight()));
		QPointF sc(tbr.width() / size.width(), tbr.height() / size.height());
		_zoom = limitZoom(OSM::scale2zoom(qMax(sc.x(), -sc.y())
		  / coordinatesRatio(), _tileSize));
	}

	return _zoom;
}

qreal OnlineMap::resolution(const QRectF &rect)
{
	return OSM::resolution(rect.center(), _zoom, _tileSize);
}

int OnlineMap::zoomIn()
{
	cancelJobs(false);

	_zoom = qMin(_zoom + 1, _zooms.max());
	return _zoom;
}

int OnlineMap::zoomOut()
{
	cancelJobs(false);

	_zoom = qMax(_zoom - 1, _zooms.min());
	return _zoom;
}

void OnlineMap::load(const Projection &in, const Projection &out,
  qreal deviceRatio, bool hidpi)
{
	Q_UNUSED(in);
	Q_UNUSED(out);

	_mapRatio = hidpi ? deviceRatio : 1.0;

	if (_scalable) {
		_scaledSize = _tileSize * deviceRatio;
		_tileRatio = deviceRatio;
	}
}

void OnlineMap::unload()
{
	cancelJobs(true);
}

qreal OnlineMap::coordinatesRatio() const
{
	return _mapRatio > 1.0 ? _mapRatio / _tileRatio : 1.0;
}

qreal OnlineMap::imageRatio() const
{
	return _mapRatio > 1.0 ? _mapRatio : _tileRatio;
}

qreal OnlineMap::tileSize() const
{
	return (_tileSize / coordinatesRatio());
}

QPoint OnlineMap::tileCoordinates(int x, int y, int zoom)
{
	return QPoint(x, _invertY ? (1<<zoom) - y - 1 : y);
}

bool OnlineMap::isRunning(const QString &key) const
{
	for (int i = 0; i < _jobs.size(); i++) {
		const QList<OnlineMapTile> &tiles = _jobs.at(i)->tiles();
		for (int j = 0; j < tiles.size(); j++)
			if (tiles.at(j).key() == key)
				return true;
	}

	return false;
}

void OnlineMap::runJob(OnlineMapJob *job)
{
	_jobs.append(job);

	connect(job, &OnlineMapJob::finished, this, &OnlineMap::jobFinished);
	job->run();
}

void OnlineMap::removeJob(OnlineMapJob *job)
{
	_jobs.removeOne(job);
	job->deleteLater();
}

void OnlineMap::jobFinished(OnlineMapJob *job)
{
	const QList<OnlineMapTile> &tiles = job->tiles();

	for (int i = 0; i < tiles.size(); i++) {
		const OnlineMapTile &mt = tiles.at(i);
		if (!mt.pixmap().isNull())
			QPixmapCache::insert(mt.key(), mt.pixmap());
	}

	removeJob(job);

	emit tilesLoaded();
}

void OnlineMap::cancelJobs(bool wait)
{
	for (int i = 0; i < _jobs.size(); i++)
		_jobs.at(i)->cancel(wait);
}

void OnlineMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	int base = _scalable ? qMin(_base, _zoom) : _zoom;
	unsigned overzoom = _zoom - base;
	unsigned f = 1U<<overzoom;

	qreal scale = OSM::zoom2scale(base, _tileSize * f);
	QPoint tile = OSM::mercator2tile(QPointF(rect.topLeft().x() * scale,
	  -rect.topLeft().y() * scale) * coordinatesRatio(), base);
	Coordinates ctl(OSM::tile2ll(tile, base));
	QPointF tl(ll2xy(Coordinates(ctl.lon(), -ctl.lat())));
	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	int width = ceil(s.width() / (tileSize() * f));
	int height = ceil(s.height() / (tileSize() * f));

	QVector<TileLoader::Tile> fetchTiles;
	fetchTiles.reserve(width * height);
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			QPoint tc(tileCoordinates(tile.x() + i, tile.y() + j, base));
			fetchTiles.append(TileLoader::Tile(tc, base));
		}
	}

	if (flags & Map::Block)
		_tileLoader->loadTilesSync(fetchTiles);
	else
		_tileLoader->loadTilesAsync(fetchTiles);

	QList<OnlineMapTile> renderTiles;
	for (int i = 0; i < fetchTiles.count(); i++) {
		const TileLoader::Tile &t = fetchTiles.at(i);
		if (t.file().isNull())
			continue;

		QString key(overzoom
		  ? t.file() + ":" + QString::number(overzoom) : t.file());
		if (isRunning(key))
			continue;

		QPixmap pm;
		if (QPixmapCache::find(key, &pm)) {
			QPointF tp(tl.x() + (t.xy().x() - tile.x()) * tileSize() * f,
			  tl.y() + (t.xy().y() - tile.y()) * tileSize() * f);
			drawTile(painter, pm, tp);
		} else
			renderTiles.append(OnlineMapTile(t.xy(), t.file(), _zoom, overzoom,
			  _scaledSize, key));
	}

	if (!renderTiles.isEmpty()) {
		if (flags & Map::Block || !_scalable) {
			QFuture<void> future = QtConcurrent::map(renderTiles,
			  &OnlineMapTile::load);
			future.waitForFinished();

			for (int i = 0; i < renderTiles.size(); i++) {
				const OnlineMapTile &mt = renderTiles.at(i);
				QPixmap pm(mt.pixmap());
				if (pm.isNull())
					continue;

				QPixmapCache::insert(mt.key(), pm);

				QPointF tp(tl.x() + (mt.xy().x() - tile.x()) * tileSize() * f,
				  tl.y() + (mt.xy().y() - tile.y()) * tileSize() * f);
				drawTile(painter, pm, tp);
			}
		} else
			runJob(new OnlineMapJob(renderTiles));
	}
}

void OnlineMap::drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp)
{
	pixmap.setDevicePixelRatio(imageRatio());
	painter->drawPixmap(tp, pixmap);
}

QPointF OnlineMap::ll2xy(const Coordinates &c)
{
	qreal scale = OSM::zoom2scale(_zoom, _tileSize);
	QPointF m = OSM::ll2m(c);
	return QPointF(m.x() / scale, m.y() / -scale) / coordinatesRatio();
}

Coordinates OnlineMap::xy2ll(const QPointF &p)
{
	qreal scale = OSM::zoom2scale(_zoom, _tileSize);
	return OSM::m2ll(QPointF(p.x() * scale, -p.y() * scale)
	  * coordinatesRatio());
}

void OnlineMap::clearCache()
{
	_tileLoader->clearCache();
	QPixmapCache::clear();
}
