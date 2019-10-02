#include <QtCore>
#include <QPainter>
#include <QDir>
#include "common/rectc.h"
#include "common/programpaths.h"
#include "downloader.h"
#include "osm.h"
#include "onlinemap.h"


OnlineMap::OnlineMap(const QString &name, const QString &url,
  const Range &zooms, const RectC &bounds, qreal tileRatio,
  const Authorization &authorization, int tileSize, bool scalable, bool invertY,
  bool quadTiles, QObject *parent)
	: Map(parent), _name(name), _zooms(zooms), _bounds(bounds),
	_zoom(_zooms.max()), _mapRatio(1.0), _tileRatio(tileRatio),
	_tileSize(tileSize), _scalable(scalable), _invertY(invertY)
{
	_tileLoader = new TileLoader(QDir(ProgramPaths::tilesDir()).filePath(_name),
	  this);
	_tileLoader->setUrl(url);
	_tileLoader->setAuthorization(authorization);
	_tileLoader->setQuadTiles(quadTiles);
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
	_zoom = qMin(_zoom + 1, _zooms.max());
	return _zoom;
}

int OnlineMap::zoomOut()
{
	_zoom = qMax(_zoom - 1, _zooms.min());
	return _zoom;
}

void OnlineMap::setDevicePixelRatio(qreal deviceRatio, qreal mapRatio)
{
	_mapRatio = mapRatio;

	if (_scalable) {
		_tileLoader->setScaledSize(_tileSize * deviceRatio);
		_tileRatio = deviceRatio;
	}
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

void OnlineMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	qreal scale = OSM::zoom2scale(_zoom, _tileSize);

	QPoint tile = OSM::mercator2tile(QPointF(rect.topLeft().x() * scale,
	  -rect.topLeft().y() * scale) * coordinatesRatio(), _zoom);
	QPointF tl(floor(rect.left() / tileSize())
	  * tileSize(), floor(rect.top() / tileSize()) * tileSize());

	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	int width = _zoom ? qCeil(s.width() / tileSize()) : 1;
	int height = _zoom ? qCeil(s.height() / tileSize()) : 1;

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
		  t.xy().y()) - tile.y()) * tileSize())
		  : QPointF(-_tileSize/2, -_tileSize/2);

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
