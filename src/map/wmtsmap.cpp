#include <QtCore>
#include <QPainter>
#include <QDir>
#include "common/rectc.h"
#include "common/wgs84.h"
#include "common/programpaths.h"
#include "transform.h"
#include "tileloader.h"
#include "wmts.h"
#include "wmtsmap.h"


#define CAPABILITIES_FILE "capabilities.xml"

WMTSMap::WMTSMap(const QString &name, const WMTS::Setup &setup, qreal tileRatio,
  QObject *parent) : Map(parent), _name(name), _tileLoader(0), _zoom(0),
  _mapRatio(1.0), _tileRatio(tileRatio)
{
	QString tilesDir(QDir(ProgramPaths::tilesDir()).filePath(_name));

	_tileLoader = new TileLoader(tilesDir, this);
	_tileLoader->setAuthorization(setup.authorization());
	connect(_tileLoader, SIGNAL(finished()), this, SIGNAL(tilesLoaded()));

	_wmts = new WMTS(QDir(tilesDir).filePath(CAPABILITIES_FILE), setup, this);
	connect(_wmts, SIGNAL(downloadFinished()), this, SLOT(wmtsReady()));
	if (_wmts->isReady())
		init();
}

void WMTSMap::init()
{
	_tileLoader->setUrl(_wmts->tileUrl());
	_bounds = RectD(_wmts->bbox(), _wmts->projection());
	updateTransform();
}

void WMTSMap::wmtsReady()
{
	if (_wmts->isValid())
		init();

	emit mapLoaded();
}

void WMTSMap::clearCache()
{
	_tileLoader->clearCache();
}

double WMTSMap::sd2res(double scaleDenominator) const
{
	return scaleDenominator * 0.28e-3
	  * _wmts->projection().units().fromMeters(1.0);
}

void WMTSMap::updateTransform()
{
	const WMTS::Zoom &z = _wmts->zooms().at(_zoom);

	PointD topLeft = (_wmts->cs().axisOrder() == CoordinateSystem::YX)
	  ? PointD(z.topLeft().y(), z.topLeft().x()) : z.topLeft();

	double pixelSpan = sd2res(z.scaleDenominator());
	if (_wmts->projection().isGeographic())
		pixelSpan /= deg2rad(WGS84_RADIUS);
	_transform = Transform(ReferencePoint(PointD(0, 0), topLeft),
	  PointD(pixelSpan, pixelSpan));
}

QRectF WMTSMap::bounds()
{
	const WMTS::Zoom &z = _wmts->zooms().at(_zoom);
	QRectF tileBounds, bounds;

	tileBounds = (z.limits().isNull()) ?
	  QRectF(QPointF(0, 0), QSize(z.tile().width() * z.matrix().width(),
	  z.tile().height() * z.matrix().height()))
	  : QRectF(QPointF(z.limits().left() * z.tile().width(), z.limits().top()
	  * z.tile().height()), QSize(z.tile().width() * z.limits().width(),
	  z.tile().height() * z.limits().height()));

	if (_bounds.isValid())
		bounds = QRectF(_transform.proj2img(_bounds.topLeft())
		  / coordinatesRatio(), _transform.proj2img(
		  _bounds.bottomRight()) / coordinatesRatio());
	return bounds.isValid() ? tileBounds.intersected(bounds) : tileBounds;
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

	QVector<Tile> tiles;
	tiles.reserve((br.x() - tl.x()) * (br.y() - tl.y()));
	for (int i = tl.x(); i < br.x(); i++)
		for (int j = tl.y(); j < br.y(); j++)
			tiles.append(Tile(QPoint(i, j), z.id()));

	if (flags & Map::Block)
		_tileLoader->loadTilesSync(tiles);
	else
		_tileLoader->loadTilesAsync(tiles);

	for (int i = 0; i < tiles.count(); i++) {
		Tile &t = tiles[i];
		QPointF tp(t.xy().x() * ts.width(), t.xy().y() * ts.height());
		if (!t.pixmap().isNull()) {
#ifdef ENABLE_HIDPI
			t.pixmap().setDevicePixelRatio(imageRatio());
#endif // ENABLE_HIDPI
			painter->drawPixmap(tp, t.pixmap());
		}
	}
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
