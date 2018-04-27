#include <QDir>
#include <QPainter>
#include "common/wgs84.h"
#include "common/rectc.h"
#include "config.h"
#include "downloader.h"
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
	double sx = (_br.x() - _tl.x()) / pixelSpan;
	double sy = (_tl.y() - _br.y()) / pixelSpan;

	ReferencePoint tl(PointD(0, 0), _tl);
	ReferencePoint br(PointD(sx, sy), _br);
	_transform = Transform(tl, br);

	_bounds = QRectF(QPointF(0, 0), QSizeF(sx, sy));
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
	_tl = _projection.ll2xy(wms.boundingBox().topLeft());
	_br = _projection.ll2xy(wms.boundingBox().bottomRight());
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
  : Map(parent), _name(name), _setup(setup), _zoom(0), _block(false),
  _valid(false)
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

qreal WMSMap::resolution(const QRectF &rect) const
{
	Coordinates tl = xy2ll((rect.topLeft()));
	Coordinates br = xy2ll(rect.bottomRight());

	qreal ds = tl.distanceTo(br);
	qreal ps = QLineF(rect.topLeft(), rect.bottomRight()).length();

	return ds/ps;
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
			if (sd2res(_zooms.at(i)) < resolution)
				break;
			_zoom = i;
		}
	} else
		_zoom = _zooms.size() - 1;

	updateTransform();
	return _zoom;
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

QPointF WMSMap::ll2xy(const Coordinates &c) const
{
	return _transform.proj2img(_projection.ll2xy(c));
}

Coordinates WMSMap::xy2ll(const QPointF &p) const
{
	return _projection.xy2ll(_transform.img2proj(p));
}

void WMSMap::draw(QPainter *painter, const QRectF &rect)
{
	QPoint tl = QPoint((int)floor(rect.left() / (qreal)TILE_SIZE),
	  (int)floor(rect.top() / (qreal)TILE_SIZE));
	QPoint br = QPoint((int)ceil(rect.right() / (qreal)TILE_SIZE),
	  (int)ceil(rect.bottom() / (qreal)TILE_SIZE));

	QList<Tile> tiles;
	for (int i = tl.x(); i < br.x(); i++) {
		for (int j = tl.y(); j < br.y(); j++) {
			PointD ttl(_transform.img2proj(QPointF(i * TILE_SIZE,
			  j * TILE_SIZE)));
			PointD tbr(_transform.img2proj(QPointF(i * TILE_SIZE + TILE_SIZE
			  - 1, j * TILE_SIZE + TILE_SIZE - 1)));
			QRectF bbox = (_cs.axisOrder() == CoordinateSystem::YX)
			  ? QRectF(QPointF(tbr.y(), tbr.x()), QPointF(ttl.y(), ttl.x()))
			  : QRectF(ttl.toPointF(), tbr.toPointF());

			tiles.append(Tile(QPoint(i, j), _zoom, bbox));
		}
	}

	if (_block)
		_tileLoader->loadTilesSync(tiles);
	else
		_tileLoader->loadTilesAsync(tiles);

	for (int i = 0; i < tiles.count(); i++) {
		Tile &t = tiles[i];
		QPoint tp(t.xy().x() * TILE_SIZE, t.xy().y() * TILE_SIZE);
		if (t.pixmap().isNull())
			painter->fillRect(QRect(tp, QSize(TILE_SIZE, TILE_SIZE)),
			  _backgroundColor);
		else
			painter->drawPixmap(tp, t.pixmap());
	}
}
