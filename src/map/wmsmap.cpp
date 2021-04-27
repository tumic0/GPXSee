#include <QtCore>
#include <QDir>
#include <QPainter>
#include "common/wgs84.h"
#include "common/rectc.h"
#include "common/programpaths.h"
#include "downloader.h"
#include "tileloader.h"
#include "wmsmap.h"


#define CAPABILITIES_FILE "capabilities.xml"
#define EPSILON 1e-6

double WMSMap::sd2res(double scaleDenominator) const
{
	return scaleDenominator * _wms->projection().units().fromMeters(1.0)
	 * 0.28e-3;
}

QString WMSMap::tileUrl() const
{
	const WMS::Setup &setup = _wms->setup();

	QString url = QString("%1%2service=WMS&version=%3&request=GetMap&bbox=$bbox"
	  "&width=%4&height=%5&layers=%6&styles=%7&format=%8&transparent=true")
	  .arg(_wms->getMapUrl(), _wms->getMapUrl().contains('?') ? "&" : "?",
	  _wms->version(), QString::number(_tileSize), QString::number(_tileSize),
	  setup.layer(), setup.style(), setup.format());

	if (_wms->version() >= "1.3.0")
		url.append(QString("&CRS=%1").arg(setup.crs()));
	else
		url.append(QString("&SRS=%1").arg(setup.crs()));

	for (int i = 0; i < setup.dimensions().size(); i++) {
		const KV<QString, QString> &dim = setup.dimensions().at(i);
		url.append(QString("&%1=%2").arg(dim.key(), dim.value()));
	}

	return url;
}

void WMSMap::computeZooms()
{
	_zooms.clear();

	const RangeF &sd = _wms->scaleDenominator();
	if (sd.size() > 0) {
		double ld = log2(sd.max() - EPSILON) - log2(sd.min() + EPSILON);
		int cld = (int)ceil(ld);
		double step = ld / (double)cld;
		double lmax = log2(sd.max() - EPSILON);
		for (int i = 0; i <= cld; i++)
			_zooms.append(pow(2.0, lmax - i * step));
	} else
		_zooms.append(sd.min() + EPSILON);
}

void WMSMap::updateTransform()
{
	double pixelSpan = sd2res(_zooms.at(_zoom));
	if (_wms->projection().isGeographic())
		pixelSpan /= deg2rad(WGS84_RADIUS);
	_transform = Transform(ReferencePoint(PointD(0, 0),
	  _wms->projection().ll2xy(_wms->bbox().topLeft())),
	  PointD(pixelSpan, pixelSpan));
}

WMSMap::WMSMap(const QString &fileName, const QString &name,
  const WMS::Setup &setup, int tileSize, QObject *parent)
  : Map(fileName, parent), _name(name), _tileLoader(0), _zoom(0),
  _tileSize(tileSize), _mapRatio(1.0)
{
	QString tilesDir(QDir(ProgramPaths::tilesDir()).filePath(_name));

	_tileLoader = new TileLoader(tilesDir, this);
	_tileLoader->setAuthorization(setup.authorization());
	connect(_tileLoader, &TileLoader::finished, this, &WMSMap::tilesLoaded);

	_wms = new WMS(QDir(tilesDir).filePath(CAPABILITIES_FILE), setup, this);
	connect(_wms, &WMS::downloadFinished, this, &WMSMap::wmsReady);
	if (_wms->isReady())
		init();
}

void WMSMap::init()
{
	_tileLoader->setUrl(tileUrl());
	_bounds = RectD(_wms->bbox(), _wms->projection());
	computeZooms();
	updateTransform();
}

void WMSMap::wmsReady()
{
	if (_wms->isValid())
		init();

	emit mapLoaded();
}

void WMSMap::clearCache()
{
	_tileLoader->clearCache();
}

QRectF WMSMap::bounds()
{
	return QRectF(_transform.proj2img(_bounds.topLeft()) / _mapRatio,
	  _transform.proj2img(_bounds.bottomRight()) / _mapRatio);
}

int WMSMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (rect.isValid()) {
		RectD prect(rect, _wms->projection());
		PointD sc(prect.width() / size.width(), prect.height() / size.height());
		double resolution = qMax(qAbs(sc.x()), qAbs(sc.y()));
		if (_wms->projection().isGeographic())
			resolution *= deg2rad(WGS84_RADIUS);

		_zoom = 0;
		for (int i = 0; i < _zooms.size(); i++) {
			if (sd2res(_zooms.at(i)) < resolution / _mapRatio)
				break;
			_zoom = i;
		}
	} else
		_zoom = _zooms.size() - 1;

	updateTransform();
	return _zoom;
}

void WMSMap::setZoom(int zoom)
{
	_zoom = zoom;
	updateTransform();
}

int WMSMap::zoomIn()
{
	_zoom = qMin(_zoom + 1, _zooms.size() - 1);
	updateTransform();
	return _zoom;
}

int WMSMap::zoomOut()
{
	_zoom = qMax(_zoom - 1, 0);
	updateTransform();
	return _zoom;
}

QPointF WMSMap::ll2xy(const Coordinates &c)
{
	return _transform.proj2img(_wms->projection().ll2xy(c)) / _mapRatio;
}

Coordinates WMSMap::xy2ll(const QPointF &p)
{
	return _wms->projection().xy2ll(_transform.img2proj(p * _mapRatio));
}

qreal WMSMap::tileSize() const
{
	return (_tileSize / _mapRatio);
}

void WMSMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	QPoint tl = QPoint(qFloor(rect.left() / tileSize()),
	  qFloor(rect.top() / tileSize()));
	QPoint br = QPoint(qCeil(rect.right() / tileSize()),
	  qCeil(rect.bottom() / tileSize()));

	QVector<Tile> tiles;
	tiles.reserve((br.x() - tl.x()) * (br.y() - tl.y()));
	for (int i = tl.x(); i < br.x(); i++) {
		for (int j = tl.y(); j < br.y(); j++) {
			PointD ttl(_transform.img2proj(QPointF(i * _tileSize,
			  j * _tileSize)));
			PointD tbr(_transform.img2proj(QPointF(i * _tileSize + _tileSize,
			  j * _tileSize + _tileSize)));
			RectD bbox = (_wms->cs().axisOrder() == CoordinateSystem::YX)
			  ? RectD(PointD(tbr.y(), tbr.x()), PointD(ttl.y(), ttl.x()))
			  : RectD(ttl, tbr);

			tiles.append(Tile(QPoint(i, j), _zoom, bbox));
		}
	}

	if (flags & Map::Block)
		_tileLoader->loadTilesSync(tiles);
	else
		_tileLoader->loadTilesAsync(tiles);

	for (int i = 0; i < tiles.count(); i++) {
		Tile &t = tiles[i];
		QPointF tp(t.xy().x() * tileSize(), t.xy().y() * tileSize());
		if (!t.pixmap().isNull()) {
			t.pixmap().setDevicePixelRatio(_mapRatio);
			painter->drawPixmap(tp, t.pixmap());
		}
	}
}
