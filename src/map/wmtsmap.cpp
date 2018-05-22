#include <QPainter>
#include "common/rectc.h"
#include "common/wgs84.h"
#include "config.h"
#include "transform.h"
#include "tileloader.h"
#include "wmts.h"
#include "wmtsmap.h"


#define CAPABILITIES_FILE "capabilities.xml"

bool WMTSMap::loadWMTS()
{
	QString file = tilesDir() + "/" + CAPABILITIES_FILE;

	WMTS wmts(file, _setup);
	if (!wmts.isValid()) {
		_errorString = wmts.errorString();
		return false;
	}

	_bounds = wmts.bounds();
	_zooms = wmts.zooms();
	_projection = wmts.projection();
	_tileLoader->setUrl(wmts.tileUrl());

	if (_setup.coordinateSystem().axisOrder() == CoordinateSystem::Unknown)
		_cs = _projection.coordinateSystem();
	else
		_cs = _setup.coordinateSystem();

	updateTransform();

	return true;
}

WMTSMap::WMTSMap(const QString &name, const WMTS::Setup &setup, QObject *parent)
  : Map(parent), _name(name), _setup(setup), _zoom(0), _valid(false)
{
	if (!QDir().mkpath(tilesDir())) {
		_errorString = "Error creating tiles dir";
		return;
	}

	_tileLoader = new TileLoader(this);
	_tileLoader->setDir(tilesDir());
	_tileLoader->setAuthorization(_setup.authorization());
	connect(_tileLoader, SIGNAL(finished()), this, SIGNAL(loaded()));

	_valid = loadWMTS();
}

void WMTSMap::clearCache()
{
	_tileLoader->clearCache();
	_zoom = 0;

	if (!loadWMTS())
		qWarning("%s: %s\n", qPrintable(_name), qPrintable(_errorString));
}

QString WMTSMap::tilesDir() const
{
	return QString(TILES_DIR + "/" + _name);
}

double WMTSMap::sd2res(double scaleDenominator) const
{
	return scaleDenominator * 0.28e-3 * _projection.units().fromMeters(1.0);
}

void WMTSMap::updateTransform()
{
	const WMTS::Zoom &z = _zooms.at(_zoom);

	PointD topLeft = (_cs.axisOrder() == CoordinateSystem::YX)
	  ? PointD(z.topLeft().y(), z.topLeft().x()) : z.topLeft();

	double pixelSpan = sd2res(z.scaleDenominator());
	if (_projection.isGeographic())
		pixelSpan /= deg2rad(WGS84_RADIUS);
	PointD tileSpan(z.tile().width() * pixelSpan, z.tile().height() * pixelSpan);
	PointD bottomRight(topLeft.x() + tileSpan.x() * z.matrix().width(),
	  topLeft.y() - tileSpan.y() * z.matrix().height());

	ReferencePoint tl(PointD(0, 0), topLeft);
	ReferencePoint br(PointD(z.tile().width() * z.matrix().width(),
	  z.tile().height() * z.matrix().height()), bottomRight);
	_transform = Transform(tl, br);
}

QRectF WMTSMap::bounds() const
{
	const WMTS::Zoom &z = _zooms.at(_zoom);
	QRectF tileBounds, bounds;

	tileBounds = (z.limits().isNull()) ?
	  QRectF(QPointF(0, 0), QSize(z.tile().width() * z.matrix().width(),
	  z.tile().height() * z.matrix().height()))
	  : QRectF(QPointF(z.limits().left() * z.tile().width(), z.limits().top()
	  * z.tile().height()), QSize(z.tile().width() * z.limits().width(),
	  z.tile().height() * z.limits().height()));

	bounds = _bounds.isValid() ? QRectF(ll2xy(_bounds.topLeft()),
	  ll2xy(_bounds.bottomRight())) : QRectF();

	return _bounds.isValid() ? tileBounds.intersected(bounds) : tileBounds;
}

int WMTSMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (rect.isValid()) {
		PointD tl(_projection.ll2xy(rect.topLeft()));
		PointD br(_projection.ll2xy(rect.bottomRight()));
		PointD sc((br.x() - tl.x()) / size.width(), (tl.y() - br.y())
		  / size.height());
		double resolution = qMax(qAbs(sc.x()), qAbs(sc.y()));
		if (_projection.isGeographic())
			resolution *= deg2rad(WGS84_RADIUS);

		_zoom = 0;
		for (int i = 0; i < _zooms.size(); i++) {
			if (sd2res(_zooms.at(i).scaleDenominator()) < resolution)
				break;
			_zoom = i;
		}
	} else
		_zoom = _zooms.size() - 1;

	updateTransform();
	return _zoom;
}

qreal WMTSMap::resolution(const QRectF &rect) const
{
	Coordinates tl = xy2ll((rect.topLeft()));
	Coordinates br = xy2ll(rect.bottomRight());

	qreal ds = tl.distanceTo(br);
	qreal ps = QLineF(rect.topLeft(), rect.bottomRight()).length();

	return ds/ps;
}

void WMTSMap::setZoom(int zoom)
{
	_zoom = zoom;
	updateTransform();
}

int WMTSMap::zoomIn()
{
	_zoom = qMin(_zoom + 1, _zooms.size() - 1);
	updateTransform();
	return _zoom;
}

int WMTSMap::zoomOut()
{
	_zoom = qMax(_zoom - 1, 0);
	updateTransform();
	return _zoom;
}

void WMTSMap::draw(QPainter *painter, const QRectF &rect, bool block)
{
	const WMTS::Zoom &z = _zooms.at(_zoom);
	QPoint tl = QPoint((int)floor(rect.left() / (qreal)z.tile().width()),
	  (int)floor(rect.top() / (qreal)z.tile().height()));
	QPoint br = QPoint((int)ceil(rect.right() / (qreal)z.tile().width()),
	  (int)ceil(rect.bottom() / (qreal)z.tile().height()));

	QList<Tile> tiles;
	for (int i = tl.x(); i < br.x(); i++)
		for (int j = tl.y(); j < br.y(); j++)
			tiles.append(Tile(QPoint(i, j), z.id()));

	if (block)
		_tileLoader->loadTilesSync(tiles);
	else
		_tileLoader->loadTilesAsync(tiles);

	for (int i = 0; i < tiles.count(); i++) {
		Tile &t = tiles[i];
		QPoint tp(t.xy().x() * z.tile().width(), t.xy().y() * z.tile().height());
		if (!t.pixmap().isNull())
			painter->drawPixmap(tp, t.pixmap());
	}
}

QPointF WMTSMap::ll2xy(const Coordinates &c) const
{
	return _transform.proj2img(_projection.ll2xy(c));
}

Coordinates WMTSMap::xy2ll(const QPointF &p) const
{
	return _projection.xy2ll(_transform.img2proj(p));
}
