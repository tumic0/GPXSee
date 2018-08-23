#include <QDir>
#include <QPainter>
#include "common/wgs84.h"
#include "common/rectc.h"
#include "config.h"
#include "downloader.h"
#include "tileloader.h"
#include "wmsmap.h"


#define CAPABILITIES_FILE "capabilities.xml"
#define TILE_SIZE 256

double WMSMap::sd2res(double scaleDenominator) const
{
	return scaleDenominator * 0.28e-3 * _projection.units().fromMeters(1.0);
}

QString WMSMap::tileUrl(const QString &version) const
{
	QString url;

	url = QString("%1?version=%2&request=GetMap&bbox=$bbox"
	  "&width=%3&height=%4&layers=%5&styles=%6&format=%7&transparent=true")
	  .arg(_setup.url(), version, QString::number(TILE_SIZE),
	  QString::number(TILE_SIZE), _setup.layer(), _setup.style(),
	  _setup.format());

	if (version >= "1.3.0")
		url.append(QString("&CRS=%1").arg(_setup.crs()));
	else
		url.append(QString("&SRS=%1").arg(_setup.crs()));

	for (int i = 0; i < _setup.dimensions().size(); i++) {
		const QPair<QString, QString> &dim = _setup.dimensions().at(i);
		url.append(QString("&%1=%2").arg(dim.first, dim.second));
	}

	return url;
}

QString WMSMap::tilesDir() const
{
	return QString(TILES_DIR + "/" + _name);
}

void WMSMap::computeZooms(const RangeF &scaleDenominator)
{
	_zooms.clear();

	if (scaleDenominator.size() > 0) {
		double ld = log2(scaleDenominator.max()) - log2(scaleDenominator.min());
		int cld = ceil(ld);
		double step = ld / (qreal)cld;
		qreal lmax = log2(scaleDenominator.max());
		for (int i = 0; i <= cld; i++)
			_zooms.append(pow(2.0, lmax - i * step));
	} else
		_zooms.append(scaleDenominator.min());
}

void WMSMap::updateTransform()
{
	double pixelSpan = sd2res(_zooms.at(_zoom));
	if (_projection.isGeographic())
		pixelSpan /= deg2rad(WGS84_RADIUS);
	double sx = _bbox.width() / pixelSpan;
	double sy = _bbox.height() / pixelSpan;

	ReferencePoint tl(PointD(0, 0), _bbox.topLeft());
	ReferencePoint br(PointD(sx, sy), _bbox.bottomRight());
	_transform = Transform(tl, br);
}

bool WMSMap::loadWMS()
{
	QString file = tilesDir() + "/" + CAPABILITIES_FILE;

	WMS wms(file, _setup);
	if (!wms.isValid()) {
		_errorString = wms.errorString();
		return false;
	}

	_projection = wms.projection();
	_bbox = RectD(_projection.ll2xy(wms.boundingBox().topLeft()),
	  _projection.ll2xy(wms.boundingBox().bottomRight()));
	_tileLoader->setUrl(tileUrl(wms.version()));

	if (wms.version() >= "1.3.0") {
		if (_setup.coordinateSystem().axisOrder() == CoordinateSystem::Unknown)
			_cs = _projection.coordinateSystem();
		else
			_cs = _setup.coordinateSystem();
	} else
		_cs = CoordinateSystem::XY;

	computeZooms(wms.scaleDenominator());
	updateTransform();

	return true;
}

WMSMap::WMSMap(const QString &name, const WMS::Setup &setup, QObject *parent)
  : Map(parent), _name(name), _setup(setup), _tileLoader(0), _zoom(0),
  _ratio(1.0), _valid(false)
{
	if (!QDir().mkpath(tilesDir())) {
		_errorString = "Error creating tiles dir";
		return;
	}

	_tileLoader = new TileLoader(this);
	_tileLoader->setDir(tilesDir());
	_tileLoader->setAuthorization(_setup.authorization());
	connect(_tileLoader, SIGNAL(finished()), this, SIGNAL(loaded()));

	_valid = loadWMS();
}

void WMSMap::clearCache()
{
	_tileLoader->clearCache();
	_zoom = 0;

	if (!loadWMS())
		qWarning("%s: %s\n", qPrintable(_name), qPrintable(_errorString));
}

QRectF WMSMap::bounds()
{
	return QRectF(_transform.proj2img(_bbox.topLeft()) / _ratio,
	  _transform.proj2img(_bbox.bottomRight()) / _ratio);
}

int WMSMap::zoomFit(const QSize &size, const RectC &rect)
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
			if (sd2res(_zooms.at(i)) < resolution / _ratio)
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
	return _transform.proj2img(_projection.ll2xy(c)) / _ratio;
}

Coordinates WMSMap::xy2ll(const QPointF &p)
{
	return _projection.xy2ll(_transform.img2proj(p * _ratio));
}

qreal WMSMap::tileSize() const
{
	return (TILE_SIZE / _ratio);
}

void WMSMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	QPoint tl = QPoint((int)floor(rect.left() / tileSize()),
	  (int)floor(rect.top() / tileSize()));
	QPoint br = QPoint((int)ceil(rect.right() / tileSize()),
	  (int)ceil(rect.bottom() / tileSize()));

	QList<Tile> tiles;
	for (int i = tl.x(); i < br.x(); i++) {
		for (int j = tl.y(); j < br.y(); j++) {
			PointD ttl(_transform.img2proj(QPointF(i * TILE_SIZE,
			  j * TILE_SIZE)));
			PointD tbr(_transform.img2proj(QPointF(i * TILE_SIZE + TILE_SIZE
			  - 1, j * TILE_SIZE + TILE_SIZE - 1)));
			RectD bbox = (_cs.axisOrder() == CoordinateSystem::YX)
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
#ifdef ENABLE_HIDPI
			t.pixmap().setDevicePixelRatio(_ratio);
#endif // ENABLE_HIDPI
			painter->drawPixmap(tp, t.pixmap());
		}
	}
}
