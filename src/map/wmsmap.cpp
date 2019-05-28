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
#define TILE_SIZE 256
#define EPSILON 1e-6

double WMSMap::sd2res(double scaleDenominator) const
{
	return scaleDenominator * 0.28e-3 * _projection.units().fromMeters(1.0);
}

QString WMSMap::tileUrl(const QString &version) const
{
	QString url;

	url = QString("%1%2version=%3&request=GetMap&bbox=$bbox"
	  "&width=%4&height=%5&layers=%6&styles=%7&format=%8&transparent=true")
	  .arg(_setup.url(), _setup.url().contains('?') ? "&" : "?", version,
	  QString::number(TILE_SIZE), QString::number(TILE_SIZE), _setup.layer(),
	  _setup.style(), _setup.format());

	if (version >= "1.3.0")
		url.append(QString("&CRS=%1").arg(_setup.crs()));
	else
		url.append(QString("&SRS=%1").arg(_setup.crs()));

	for (int i = 0; i < _setup.dimensions().size(); i++) {
		const KV &dim = _setup.dimensions().at(i);
		url.append(QString("&%1=%2").arg(dim.key(), dim.value()));
	}

	return url;
}

QString WMSMap::tilesDir() const
{
	return QString(QDir(ProgramPaths::tilesDir()).filePath(_name));
}

void WMSMap::computeZooms(const RangeF &scaleDenominator)
{
	_zooms.clear();

	if (scaleDenominator.size() > 0) {
		double ld = log2(scaleDenominator.max() - EPSILON)
		  - log2(scaleDenominator.min() + EPSILON);
		int cld = (int)ceil(ld);
		double step = ld / (double)cld;
		double lmax = log2(scaleDenominator.max() - EPSILON);
		for (int i = 0; i <= cld; i++)
			_zooms.append(pow(2.0, lmax - i * step));
	} else
		_zooms.append(scaleDenominator.min() + EPSILON);
}

void WMSMap::updateTransform()
{
	double pixelSpan = sd2res(_zooms.at(_zoom));
	if (_projection.isGeographic())
		pixelSpan /= deg2rad(WGS84_RADIUS);
	_transform = Transform(ReferencePoint(PointD(0, 0),
	  _projection.ll2xy(_bbox.topLeft())), PointD(pixelSpan, pixelSpan));
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
	_bbox = wms.boundingBox();
	_bounds = RectD(_bbox, _projection);
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
  _mapRatio(1.0), _valid(false)
{
	_tileLoader = new TileLoader(tilesDir(), this);
	_tileLoader->setAuthorization(_setup.authorization());
	connect(_tileLoader, SIGNAL(finished()), this, SIGNAL(loaded()));

	_valid = loadWMS();
}

void WMSMap::clearCache()
{
	_tileLoader->clearCache();
	_zoom = 0;

	if (!loadWMS())
		qWarning("%s: %s", qPrintable(_name), qPrintable(_errorString));
}

QRectF WMSMap::bounds()
{
	return QRectF(_transform.proj2img(_bounds.topLeft()) / _mapRatio,
	  _transform.proj2img(_bounds.bottomRight()) / _mapRatio);
}

int WMSMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (rect.isValid()) {
		RectD prect(rect, _projection);
		PointD sc(prect.width() / size.width(), prect.height() / size.height());
		double resolution = qMax(qAbs(sc.x()), qAbs(sc.y()));
		if (_projection.isGeographic())
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
	return _transform.proj2img(_projection.ll2xy(c)) / _mapRatio;
}

Coordinates WMSMap::xy2ll(const QPointF &p)
{
	return _projection.xy2ll(_transform.img2proj(p * _mapRatio));
}

qreal WMSMap::tileSize() const
{
	return (TILE_SIZE / _mapRatio);
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
			PointD ttl(_transform.img2proj(QPointF(i * TILE_SIZE,
			  j * TILE_SIZE)));
			PointD tbr(_transform.img2proj(QPointF(i * TILE_SIZE + TILE_SIZE,
			  j * TILE_SIZE + TILE_SIZE)));
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
			t.pixmap().setDevicePixelRatio(_mapRatio);
#endif // ENABLE_HIDPI
			painter->drawPixmap(tp, t.pixmap());
		}
	}
}
