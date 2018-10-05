#include <QPainter>
#include "common/rectc.h"
#include "downloader.h"
#include "osm.h"
#include "config.h"
#include "onlinemap.h"


#define TILE_SIZE     256

OnlineMap::OnlineMap(const QString &name, const QString &url,
  const Range &zooms, const RectC &bounds, qreal tileRatio,
  const Authorization &authorization, bool invertY, QObject *parent)
	: Map(parent), _name(name), _zooms(zooms), _bounds(bounds),
	_zoom(_zooms.max()), _deviceRatio(1.0), _tileRatio(tileRatio),
	_invertY(invertY)
{
	_tileLoader = new TileLoader(TILES_DIR + "/" + _name, this);
	_tileLoader->setUrl(url);
	_tileLoader->setAuthorization(authorization);
	connect(_tileLoader, SIGNAL(finished()), this, SIGNAL(loaded()));
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
		  / coordinatesRatio(), TILE_SIZE));
	}

	return _zoom;
}

qreal OnlineMap::resolution(const QRectF &rect)
{
	return OSM::resolution(rect.center(), _zoom, TILE_SIZE);
}

int OnlineMap::zoomIn()
{
	_zoom = qMin(_zoom + 1, _zooms.max());
	return _zoom;
}

int OnlineMap::zoomOut()
{
	_zoom = qMax(_zoom - 1, _zooms.min());
	return _zoom;
}

qreal OnlineMap::coordinatesRatio() const
{
	return _deviceRatio > 1.0 ? _deviceRatio / _tileRatio : 1.0;
}

qreal OnlineMap::imageRatio() const
{
	return _deviceRatio > 1.0 ? _deviceRatio : _tileRatio;
}

qreal OnlineMap::tileSize() const
{
	return (TILE_SIZE / coordinatesRatio());
}

void OnlineMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	qreal scale = OSM::zoom2scale(_zoom, TILE_SIZE);
	QRectF b(bounds());

	QPoint tile = OSM::mercator2tile(QPointF(rect.topLeft().x() * scale,
	  -rect.topLeft().y() * scale) * coordinatesRatio(), _zoom);
	QPointF tl(floor(rect.left() / tileSize())
	  * tileSize(), floor(rect.top() / tileSize()) * tileSize());

	QSizeF s(qMin(rect.right() - tl.x(), b.width()),
	  qMin(rect.bottom() - tl.y(), b.height()));
	int width = ceil(s.width() / tileSize());
	int height = ceil(s.height() / tileSize());

	QVector<Tile> tiles;
	tiles.reserve(width * height);
	for (int i = 0; i < width; i++)
		for (int j = 0; j < height; j++)
			tiles.append(Tile(QPoint(tile.x() + i, _invertY ? (1<<_zoom)
			  - (tile.y() + j) - 1 : tile.y() + j), _zoom));

	if (flags & Map::Block)
		_tileLoader->loadTilesSync(tiles);
	else
		_tileLoader->loadTilesAsync(tiles);

	for (int i = 0; i < tiles.count(); i++) {
		Tile &t = tiles[i];
		QPointF tp = _zoom ? QPointF(tl.x() + (t.xy().x() - tile.x())
		  * tileSize(), tl.y() + ((_invertY ? (1<<_zoom) - t.xy().y() - 1 :
		  t.xy().y()) - tile.y()) * tileSize()) : QPointF(-128, -128);

		if (!t.pixmap().isNull()) {
#ifdef ENABLE_HIDPI
			t.pixmap().setDevicePixelRatio(imageRatio());
#endif // ENABLE_HIDPI
			painter->drawPixmap(tp, t.pixmap());
		}
	}
}

QPointF OnlineMap::ll2xy(const Coordinates &c)
{
	qreal scale = OSM::zoom2scale(_zoom, TILE_SIZE);
	QPointF m = OSM::ll2m(c);
	return QPointF(m.x() / scale, m.y() / -scale) / coordinatesRatio();
}

Coordinates OnlineMap::xy2ll(const QPointF &p)
{
	qreal scale = OSM::zoom2scale(_zoom, TILE_SIZE);
	return OSM::m2ll(QPointF(p.x() * scale, -p.y() * scale)
	  * coordinatesRatio());
}
