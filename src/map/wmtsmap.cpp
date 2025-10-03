#include <QtMath>
#include <QPainter>
#include <QDir>
#include <QPixmapCache>
#include <QtConcurrent>
#include "common/rectc.h"
#include "common/wgs84.h"
#include "common/programpaths.h"
#include "transform.h"
#include "tileloader.h"
#include "tile.h"
#include "wmts.h"
#include "wmtsmap.h"


#define CAPABILITIES_FILE "capabilities.xml"

WMTSMap::WMTSMap(const QString &fileName, const QString &name,
  const WMTS::Setup &setup, qreal tileRatio,
  QObject *parent) : Map(fileName, parent), _name(name), _tileLoader(0),
  _zoom(0), _mapRatio(1.0), _tileRatio(tileRatio)
{
	QString tilesDir(QDir(ProgramPaths::tilesDir()).filePath(_name));

	_tileLoader = new TileLoader(tilesDir, this);
	_tileLoader->setHeaders(setup.headers());
	connect(_tileLoader, &TileLoader::finished, this, &WMTSMap::tilesLoaded);

	_wmts = new WMTS(QDir(tilesDir).filePath(CAPABILITIES_FILE), setup, this);
	connect(_wmts, &WMTS::downloadFinished, this, &WMTSMap::wmtsReady);
	if (_wmts->isReady())
		init();
}

void WMTSMap::init()
{
	_tileLoader->setUrl(_wmts->tileUrl(), TileLoader::XYZ);
	_bounds = RectD(_wmts->bbox(), _wmts->projection());
	updateTransform();
}

void WMTSMap::wmtsReady()
{
	if (_wmts->isValid())
		init();

	emit mapLoaded();
}

void WMTSMap::load(const Projection &in, const Projection &out,
  qreal deviceRatio, bool hidpi, int style, int layer)
{
	Q_UNUSED(in);
	Q_UNUSED(out);
	Q_UNUSED(style);
	Q_UNUSED(layer);

	_mapRatio = hidpi ? deviceRatio : 1.0;
}

void WMTSMap::clearCache()
{
	_tileLoader->clearCache();
	QPixmapCache::clear();
}

double WMTSMap::sd2res(double scaleDenominator) const
{
	return scaleDenominator * 0.28e-3
	  * _wmts->projection().units().fromMeters(1.0);
}

Transform WMTSMap::transform(int zoom) const
{
	const WMTS::Zoom &z = _wmts->zooms().at(zoom);

	PointD topLeft = (_wmts->cs().axisOrder() == CoordinateSystem::YX)
	  ? PointD(z.topLeft().y(), z.topLeft().x()) : z.topLeft();

	double pixelSpan = sd2res(z.scaleDenominator());
	if (_wmts->projection().isGeographic())
		pixelSpan /= deg2rad(WGS84_RADIUS);
	return Transform(ReferencePoint(PointD(0, 0), topLeft),
	  PointD(pixelSpan, pixelSpan));
}

QRectF WMTSMap::tileBounds(int zoom) const
{
	const WMTS::Zoom &z = _wmts->zooms().at(zoom);

	return (z.limits().isNull())
	  ? QRectF(QPointF(0, 0), QSize(z.tile().width() * z.matrix().width(),
	  z.tile().height() * z.matrix().height()))
	  : QRectF(QPointF(z.limits().left() * z.tile().width(), z.limits().top()
	  * z.tile().height()), QSize(z.tile().width() * z.limits().width(),
	  z.tile().height() * z.limits().height()));
}

void WMTSMap::updateTransform()
{
	_transform = transform(_zoom);
}

QRectF WMTSMap::bounds()
{
	QRectF tb(tileBounds(_zoom));
	QRectF lb = _bounds.isValid()
	  ? QRectF(_transform.proj2img(_bounds.topLeft()) / coordinatesRatio(),
	  _transform.proj2img(_bounds.bottomRight()) / coordinatesRatio())
	  : QRectF();

	return lb.isValid() ? lb & tb : tb;
}

RectC WMTSMap::llBounds()
{
	if (_wmts->bbox().isValid())
		return _wmts->bbox();
	else {
		int maxZoom = _wmts->zooms().size() - 1;
		QRectF tb(tileBounds(maxZoom));
		Transform t(transform(maxZoom));
		RectD rect(t.img2proj(tb.topLeft() * coordinatesRatio()),
		  t.img2proj(tb.bottomRight() * coordinatesRatio()));
		return rect.toRectC(_wmts->projection());
	}
}

int WMTSMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (rect.isValid()) {
		RectD prect(rect, _wmts->projection());
		PointD sc(prect.width() / size.width(), prect.height() / size.height());
		double resolution = qMax(qAbs(sc.x()), qAbs(sc.y()));
		if (_wmts->projection().isGeographic())
			resolution *= deg2rad(WGS84_RADIUS);

		_zoom = 0;
		for (int i = 0; i < _wmts->zooms().size(); i++) {
			if (sd2res(_wmts->zooms().at(i).scaleDenominator()) < resolution
			  / coordinatesRatio())
				break;
			_zoom = i;
		}
	} else
		_zoom = _wmts->zooms().size() - 1;

	updateTransform();
	return _zoom;
}

void WMTSMap::setZoom(int zoom)
{
	_zoom = zoom;
	updateTransform();
}

int WMTSMap::zoomIn()
{
	_zoom = qMin(_zoom + 1, _wmts->zooms().size() - 1);
	updateTransform();
	return _zoom;
}

int WMTSMap::zoomOut()
{
	_zoom = qMax(_zoom - 1, 0);
	updateTransform();
	return _zoom;
}

qreal WMTSMap::coordinatesRatio() const
{
	return _mapRatio > 1.0 ? _mapRatio / _tileRatio : 1.0;
}

qreal WMTSMap::imageRatio() const
{
	return _mapRatio > 1.0 ? _mapRatio : _tileRatio;
}

QSizeF WMTSMap::tileSize(const WMTS::Zoom &zoom) const
{
	return QSizeF(zoom.tile().width() / coordinatesRatio(),
	  zoom.tile().height() / coordinatesRatio());
}

void WMTSMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	const WMTS::Zoom &z = _wmts->zooms().at(_zoom);
	QSizeF ts(tileSize(z));

	QPoint tl = QPoint(qFloor(rect.left() / ts.width()),
	  qFloor(rect.top() / ts.height()));
	QPoint br = QPoint(qCeil(rect.right() / ts.width()),
	  qCeil(rect.bottom() / ts.height()));

	QVector<TileLoader::Tile> fetchTiles;
	fetchTiles.reserve((br.x() - tl.x()) * (br.y() - tl.y()));
	for (int i = tl.x(); i < br.x(); i++)
		for (int j = tl.y(); j < br.y(); j++)
			fetchTiles.append(TileLoader::Tile(QPoint(i, j), z.id()));

	if (flags & Map::Block)
		_tileLoader->loadTilesSync(fetchTiles);
	else
		_tileLoader->loadTilesAsync(fetchTiles);

	QList<FileTile> renderTiles;
	for (int i = 0; i < fetchTiles.count(); i++) {
		const TileLoader::Tile &t = fetchTiles.at(i);
		if (t.file().isNull())
			continue;

		QPixmap pm;
		if (QPixmapCache::find(t.file(), &pm)) {
			QPointF tp(t.xy().x() * ts.width(), t.xy().y() * ts.height());
			drawTile(painter, pm, tp);
		} else
			renderTiles.append(FileTile(t.xy(), t.file()));
	}

	QFuture<void> future = QtConcurrent::map(renderTiles, &FileTile::load);
	future.waitForFinished();

	for (int i = 0; i < renderTiles.size(); i++) {
		const FileTile &mt = renderTiles.at(i);
		QPixmap pm(mt.pixmap());
		if (pm.isNull())
			continue;

		QPixmapCache::insert(mt.file(), pm);

		QPointF tp(mt.xy().x() * ts.width(), mt.xy().y() * ts.height());
		drawTile(painter, pm, tp);
	}
}

void WMTSMap::drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp)
{
	pixmap.setDevicePixelRatio(imageRatio());
	painter->drawPixmap(tp, pixmap);
}

QPointF WMTSMap::ll2xy(const Coordinates &c)
{
	return _transform.proj2img(_wmts->projection().ll2xy(c))
	  / coordinatesRatio();
}

Coordinates WMTSMap::xy2ll(const QPointF &p)
{
	return _wmts->projection().xy2ll(_transform.img2proj(p
	  * coordinatesRatio()));
}
