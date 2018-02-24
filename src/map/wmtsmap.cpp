#include <QPainter>
#include "common/rectc.h"
#include "common/wgs84.h"
#include "config.h"
#include "transform.h"
#include "wmts.h"
#include "wmtsmap.h"


#define CAPABILITIES_FILE "capabilities.xml"

WMTSMap::WMTSMap(const QString &name, const QString &url, const QString &format,
  const QString &layer, const QString &style, const QString &set,
  bool invertAxis, QObject *parent) :
	Map(parent), _name(name), _url(url), _layer(layer), _set(set),
	_invertAxis(invertAxis), _zoom(0), _valid(false)
{
	QString dir(TILES_DIR + "/" + _name);
	QString file = dir + "/" + CAPABILITIES_FILE;

	QString tileUrl = QString("%1?service=WMTS&Version=1.0.0&request=GetTile"
	  "&Format=%2&Layer=%3&Style=%4&TileMatrixSet=%5&TileMatrix=$z&TileRow=$y"
	  "&TileCol=$x").arg(_url).arg(format).arg(layer).arg(style)
	  .arg(_set);
	_tileLoader = TileLoader(tileUrl, dir);

	WMTS wmts;
	if (!wmts.load(file, _url, _layer, _set)) {
		_errorString = wmts.errorString();
		return;
	}
	_bounds = wmts.bounds();
	_zooms = wmts.zooms();
	_projection = wmts.projection();

	updateTransform();

	_block = false;
	_valid = true;
}

qreal WMTSMap::sd2res(qreal scaleDenominator) const
{
	return scaleDenominator * 0.28e-3 * _projection.units().fromMeters(1.0);
}

void WMTSMap::updateTransform()
{
	const WMTS::Zoom &z = _zooms.at(_zoom);
	ReferencePoint tl, br;

	qreal pixelSpan = sd2res(z.scaleDenominator);
	if (_projection.isGeographic())
		pixelSpan /= deg2rad(WGS84_RADIUS);
	QPointF tileSpan(z.tile.width() * pixelSpan, z.tile.height() * pixelSpan);
	QPointF topLeft = _invertAxis
	  ? QPointF(z.topLeft.y(), z.topLeft.x()) : z.topLeft;
	QPointF bottomRight(topLeft.x() + tileSpan.x() * z.matrix.width(),
	  topLeft.y() - tileSpan.y() * z.matrix.height());

	tl.xy = QPoint(0, 0);
	tl.pp = topLeft;
	br.xy = QPoint(z.tile.width() * z.matrix.width(),
	  z.tile.height() * z.matrix.height());
	br.pp = bottomRight;

	QList<ReferencePoint> points;
	points << tl << br;
	Transform tr(points);
	_transform = tr.transform();
	_inverted = _transform.inverted();
}

void WMTSMap::load()
{
	connect(TileLoader::downloader(), SIGNAL(finished()), this,
	  SLOT(emitLoaded()));
}

void WMTSMap::unload()
{
	disconnect(TileLoader::downloader(), SIGNAL(finished()), this,
	  SLOT(emitLoaded()));
}

void WMTSMap::clearCache()
{
	QString dir(TILES_DIR + "/" + _name);
	QString file = dir + "/" + CAPABILITIES_FILE;

	_tileLoader.clearCache();

	WMTS wmts;
	if (!wmts.load(file, _url, _layer, _set))
		return;
	_zooms = wmts.zooms();
	_projection = wmts.projection();

	if (_zoom >= _zooms.size())
		_zoom = _zooms.size() - 1;
	updateTransform();
}

void WMTSMap::emitLoaded()
{
	emit loaded();
}

QRectF WMTSMap::bounds() const
{
	const WMTS::Zoom &z = _zooms.at(_zoom);
	QRectF tileBounds, bounds;

	tileBounds = (z.limits.isNull()) ?
	  QRectF(QPointF(0, 0), QSize(z.tile.width() * z.matrix.width(),
	  z.tile.height() * z.matrix.height())) : QRectF(QPointF(z.limits.left()
	  * z.tile.width(), z.limits.top() * z.tile.height()), QSize(z.tile.width()
	  * z.limits.width(), z.tile.height() * z.limits.height()));

	bounds = _bounds.isValid() ? QRectF(ll2xy(_bounds.topLeft()),
	  ll2xy(_bounds.bottomRight())) : QRectF();

	return _bounds.isValid() ? tileBounds.intersected(bounds) : tileBounds;
}

qreal WMTSMap::zoomFit(const QSize &size, const RectC &br)
{
	_zoom = 0;

	if (br.isValid()) {
		QRectF tbr(_projection.ll2xy(br.topLeft()),
		  _projection.ll2xy(br.bottomRight()));
		QPointF sc(tbr.width() / size.width(), tbr.height() / size.height());
		qreal resolution = qMax(qAbs(sc.x()), qAbs(sc.y()));
		if (_projection.isGeographic())
			resolution *= deg2rad(WGS84_RADIUS);

		for (int i = 0; i < _zooms.size(); i++) {
			if (sd2res(_zooms.at(i).scaleDenominator) < resolution)
				break;
			_zoom = i;
		}
	}

	updateTransform();
	return _zoom;
}

qreal WMTSMap::zoomFit(qreal resolution, const Coordinates &c)
{
	Q_UNUSED(c);

	_zoom = 0;

	for (int i = 0; i < _zooms.size(); i++) {
		if (sd2res(_zooms.at(i).scaleDenominator) < resolution)
			break;
		_zoom = i;
	}

	updateTransform();
	return _zoom;
}

qreal WMTSMap::resolution(const QPointF &p) const
{
	Q_UNUSED(p);

	return sd2res(_zooms.at(_zoom).scaleDenominator);
}

qreal WMTSMap::zoomIn()
{
	_zoom = qMin(_zoom + 1, _zooms.size() - 1);
	updateTransform();
	return _zoom;
}

qreal WMTSMap::zoomOut()
{
	_zoom = qMax(_zoom - 1, 0);
	updateTransform();
	return _zoom;
}

void WMTSMap::draw(QPainter *painter, const QRectF &rect)
{
	const WMTS::Zoom &z = _zooms.at(_zoom);
	QPoint tl = QPoint((int)floor(rect.left() / (qreal)z.tile.width()),
	  (int)floor(rect.top() / (qreal)z.tile.height()));
	QPoint br = QPoint((int)ceil(rect.right() / (qreal)z.tile.width()),
	  (int)ceil(rect.bottom() / (qreal)z.tile.height()));

	QList<Tile> tiles;
	for (int i = tl.x(); i < br.x(); i++)
		for (int j = tl.y(); j < br.y(); j++)
			tiles.append(Tile(QPoint(i, j), z.id));

	if (_block)
		_tileLoader.loadTilesSync(tiles);
	else
		_tileLoader.loadTilesAsync(tiles);

	for (int i = 0; i < tiles.count(); i++) {
		Tile &t = tiles[i];
		QPoint tp(t.xy().x() * z.tile.width(), t.xy().y() * z.tile.height());
		if (t.pixmap().isNull())
			painter->fillRect(QRect(tp, z.tile), _backgroundColor);
		else
			painter->drawPixmap(tp, t.pixmap());
	}
}

QPointF WMTSMap::ll2xy(const Coordinates &c) const
{
	return _transform.map(_projection.ll2xy(c));
}

Coordinates WMTSMap::xy2ll(const QPointF &p) const
{
	return _projection.xy2ll(_inverted.map(p));
}
