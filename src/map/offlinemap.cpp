#include <QtGlobal>
#include <QPainter>
#include <QFileInfo>
#include <QMap>
#include <QDir>
#include <QBuffer>
#include <QImage>
#include <QImageReader>
#include <QPixmapCache>
#include "common/coordinates.h"
#include "common/rectc.h"
#include "mapfile.h"
#include "geotiff.h"
#include "offlinemap.h"


void OfflineMap::computeResolution()
{
	Coordinates tl = xy2ll((bounds().topLeft()));
	Coordinates br = xy2ll(bounds().bottomRight());

	qreal ds = tl.distanceTo(br);
	qreal ps = QLineF(bounds().topLeft(), bounds().bottomRight()).length();

	_resolution = ds/ps;
}

bool OfflineMap::getImageInfo(const QString &path)
{
	QFileInfo ii(_imgPath);

	if (ii.isRelative())
		ii.setFile(path + "/" + _imgPath);

	if (!ii.exists()) {
		int last = _imgPath.lastIndexOf('\\');
		if (last >= 0 && last < _imgPath.length() - 1) {
			QStringRef fn(&_imgPath, last + 1, _imgPath.length() - last - 1);
			ii.setFile(path + "/" + fn.toString());
		}
	}

	if (ii.exists())
		_imgPath = ii.absoluteFilePath();
	else {
		_errorString = QString("%1: No such image file").arg(_imgPath);
		return false;
	}

	if (OZF::isOZF(_imgPath)) {
		if (!_ozf.load(_imgPath)) {
			_errorString = QString("%1: Error loading OZF file")
			  .arg(QFileInfo(_imgPath).fileName());
			return false;
		}
	} else {
		QImageReader img(_imgPath);
		_size = img.size();
		if (!_size.isValid()) {
			_errorString = QString("%1: Error reading map image")
			  .arg(QFileInfo(_imgPath).fileName());
			return false;
		}
	}

	return true;
}

bool OfflineMap::getTileInfo(const QStringList &tiles, const QString &path)
{
	QRegExp rx("_[0-9]+_[0-9]+\\.");
	for (int i = 0; i < tiles.size(); i++) {
		if (tiles.at(i).contains(rx)) {
			_tileName = QString(tiles.at(i)).replace(rx, "_%1_%2.");

			if (path.isNull()) {
				QByteArray ba = _tar.file(tiles.at(i));
				QBuffer buffer(&ba);
				_tileSize = QImageReader(&buffer).size();
			} else {
				_tileName = path + "/" + _tileName;
				_tileSize = QImageReader(path + "/" + tiles.at(i)).size();
			}
			if (!_tileSize.isValid()) {
				_errorString = QString("Error retrieving tile size: "
				  "%1: Invalid image").arg(QFileInfo(tiles.at(i)).fileName());
				return false;
			}

			return true;
		}
	}

	_errorString = "Invalid/missing tile set";
	return false;
}

bool OfflineMap::totalSizeSet()
{
	if (!_size.isValid()) {
		_errorString = "Missing total image size (IWH)";
		return false;
	} else
		return true;
}

OfflineMap::OfflineMap(const QString &fileName, QObject *parent)
  : Map(parent)
{
	QFileInfo fi(fileName);
	QString suffix = fi.suffix().toLower();

	_valid = false;
	_img = 0;
	_projection = 0;
	_resolution = 0.0;
	_zoom = 0;
	_scale = QPointF(1.0, 1.0);

	if (suffix == "tar") {
		if (!_tar.load(fileName)) {
			_errorString = "Error reading tar file";
			return;
		}

		QString mapFileName = fi.completeBaseName() + ".map";
		QByteArray ba = _tar.file(mapFileName);
		if (ba.isNull()) {
			_errorString = "Map file not found";
			return;
		}
		QBuffer mapFile(&ba);
		MapFile mf;
		if (!mf.load(mapFile)) {
			_errorString = mf.errorString();
			return;
		} else {
			_name = mf.name();
			_size = mf.size();
			_imgPath = mf.image();
			_gcs = mf.gcs();
			_projection = mf.projection();
			_transform = mf.transform();
		}
	} else if (suffix == "map") {
		MapFile mf;
		QFile mapFile(fileName);
		if (!mf.load(mapFile)) {
			_errorString = mf.errorString();
			return;
		} else {
			_name = mf.name();
			_size = mf.size();
			_imgPath = mf.image();
			_gcs = mf.gcs();
			_projection = mf.projection();
			_transform = mf.transform();
		}
	} else if (suffix == "tif" || suffix == "tiff") {
		GeoTIFF gt;
		if (!gt.load(fileName)) {
			_errorString = gt.errorString();
			return;
		} else {
			_name = fi.fileName();
			_imgPath = fileName;
			_gcs = gt.gcs();
			_projection = gt.projection();
			_transform = gt.transform();
		}
	} else {
		_errorString = "Not a map file";
		return;
	}

	if (_tar.isOpen()) {
		if (!totalSizeSet())
			return;
		if (!getTileInfo(_tar.files()))
			return;
		_imgPath = QString();
	} else {
		QDir set(fi.absolutePath() + "/" + "set");
		if (set.exists()) {
			if (!totalSizeSet())
				return;
			if (!getTileInfo(set.entryList(), set.absolutePath()))
				return;
			_imgPath = QString();
		} else {
			if (!getImageInfo(fi.absolutePath()))
				return;
		}
	}

	_inverted = _transform.inverted();
	computeResolution();

	_valid = true;
}

OfflineMap::OfflineMap(const QString &fileName, Tar &tar, QObject *parent)
  : Map(parent)
{
	QFileInfo fi(fileName);
	MapFile mf;

	_valid = false;
	_img = 0;
	_projection = 0;
	_resolution = 0.0;
	_zoom = 0;
	_scale = QPointF(1.0, 1.0);

	QFileInfo map(fi.absolutePath());
	QFileInfo layer(map.absolutePath());
	QString mapFile = layer.fileName() + "/" + map.fileName() + "/"
	  + fi.fileName();
	QByteArray ba = tar.file(mapFile);
	if (ba.isNull()) {
		_errorString = "Map file not found";
		return;
	}
	QBuffer buffer(&ba);

	if (!mf.load(buffer)) {
		_errorString = mf.errorString();
		return;
	}

	_name = mf.name();
	_size = mf.size();
	_gcs = mf.gcs();
	_projection = mf.projection();
	_transform = mf.transform();

	_inverted = _transform.inverted();
	computeResolution();

	_tarPath = fi.absolutePath() + "/" + fi.completeBaseName() + ".tar";
	_valid = true;
}

OfflineMap::~OfflineMap()
{
	delete _img;
	delete _projection;
}

void OfflineMap::load()
{
	if (!_tarPath.isNull() && !_tileSize.isValid()) {
		if (!_tar.load(_tarPath)) {
			qWarning("%s: error loading tar file", qPrintable(_tarPath));
			return;
		}
		if (!getTileInfo(_tar.files()))
			qWarning("%s: %s", qPrintable(_tarPath), qPrintable(_errorString));
		return;
	}

	if (!_img && !_imgPath.isNull() && !_ozf.isOpen()) {
		_img = new QImage(_imgPath);
		if (_img->isNull())
			qWarning("%s: error loading map image", qPrintable(_imgPath));
	}
}

void OfflineMap::unload()
{
	delete _img;
	_img = 0;
}

void OfflineMap::drawTiled(QPainter *painter, const QRectF &rect)
{
	QPoint tl = QPoint((int)floor(rect.left() / (qreal)_tileSize.width())
	  * _tileSize.width(), (int)floor(rect.top() / _tileSize.height())
	  * _tileSize.height());

	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	for (int i = 0; i < ceil(s.width() / _tileSize.width()); i++) {
		for (int j = 0; j < ceil(s.height() / _tileSize.height()); j++) {
			int x = tl.x() + i * _tileSize.width();
			int y = tl.y() + j * _tileSize.height();

			QString tileName(_tileName.arg(QString::number(x),
			  QString::number(y)));
			QPixmap pixmap;

			if (_tar.isOpen()) {
				QString key = _tar.fileName() + "/" + tileName;
				if (!QPixmapCache::find(key, &pixmap)) {
					QByteArray ba = _tar.file(tileName);
					pixmap = QPixmap::fromImage(QImage::fromData(ba));
					if (!pixmap.isNull())
						QPixmapCache::insert(key, pixmap);
				}
			} else
				pixmap = QPixmap(tileName);

			if (pixmap.isNull()) {
				qWarning("%s: error loading tile image", qPrintable(
				  _tileName.arg(QString::number(x), QString::number(y))));
				painter->fillRect(QRectF(QPoint(x, y), _tileSize),
				  _backgroundColor);
			} else
				painter->drawPixmap(QPoint(x, y), pixmap);
		}
	}
}

void OfflineMap::drawOZF(QPainter *painter, const QRectF &rect)
{
	QPoint tl = QPoint((int)floor(rect.left() / _ozf.tileSize().width())
	  * _ozf.tileSize().width(), (int)floor(rect.top()
	  / _ozf.tileSize().height()) * _ozf.tileSize().height());

	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	for (int i = 0; i < ceil(s.width() / _ozf.tileSize().width()); i++) {
		for (int j = 0; j < ceil(s.height() / _ozf.tileSize().height()); j++) {
			int x = tl.x() + i * _ozf.tileSize().width();
			int y = tl.y() + j * _ozf.tileSize().height();

			QPixmap pixmap;
			QString key = _ozf.fileName() + "/" + QString::number(_zoom) + "_"
			  + QString::number(x) + "_" + QString::number(y);
			if (!QPixmapCache::find(key, &pixmap)) {
				pixmap = _ozf.tile(_zoom, x, y);
				if (!pixmap.isNull())
					QPixmapCache::insert(key, pixmap);
			}

			if (pixmap.isNull()) {
				qWarning("%s: error loading tile image", qPrintable(key));
				painter->fillRect(QRectF(QPoint(x, y), _ozf.tileSize()),
				  _backgroundColor);
			} else
				painter->drawPixmap(QPoint(x, y), pixmap);
		}
	}
}

void OfflineMap::drawImage(QPainter *painter, const QRectF &rect)
{
	QRect r(rect.toRect());
	painter->drawImage(r.left(), r.top(), *_img, r.left(), r.top(),
	  r.width(), r.height());
}

void OfflineMap::draw(QPainter *painter, const QRectF &rect)
{
	if (_ozf.isOpen())
		drawOZF(painter, rect);
	else if (_tileSize.isValid())
		drawTiled(painter, rect);
	else if (_img && !_img->isNull())
		drawImage(painter, rect);
	else
		painter->fillRect(rect, _backgroundColor);
}

QPointF OfflineMap::ll2xy(const Coordinates &c)
{
	if (_ozf.isOpen()) {
		QPointF p(_transform.map(_projection->ll2xy(_gcs->fromWGS84(c))));
		return QPointF(p.x() * _scale.x(), p.y() * _scale.y());
	} else
		return _transform.map(_projection->ll2xy(_gcs->fromWGS84(c)));
}

Coordinates OfflineMap::xy2ll(const QPointF &p)
{
	if (_ozf.isOpen()) {
		return _gcs->toWGS84(_projection->xy2ll(_inverted.map(QPointF(p.x()
		  / _scale.x(), p.y() / _scale.y()))));
	} else
		return _gcs->toWGS84(_projection->xy2ll(_inverted.map(p)));
}

QRectF OfflineMap::bounds() const
{
	if (_ozf.isOpen())
		return QRectF(QPointF(0, 0), _ozf.size(_zoom));
	else
		return QRectF(QPointF(0, 0), _size);
}

qreal OfflineMap::resolution(const QPointF &p) const
{
	Q_UNUSED(p);

	if (_ozf.isOpen())
		return _resolution / ((_scale.x() + _scale.y()) / 2.0);
	else
		return _resolution;
}

qreal OfflineMap::zoomFit(const QSize &size, const RectC &br)
{
	if (_ozf.isOpen()) {
		if (!br.isValid())
			rescale(0);
		else {
			QRect sbr(QRectF(_transform.map(_projection->ll2xy(br.topLeft())),
			  _transform.map(_projection->ll2xy(br.bottomRight())))
			  .toRect().normalized());

			for (int i = 0; i < _ozf.zooms(); i++) {
				rescale(i);
				if (sbr.size().width() * _scale.x() <= size.width()
				  && sbr.size().height() * _scale.y() <= size.height())
					break;
			}
		}
	}

	return _zoom;
}

qreal OfflineMap::zoomFit(qreal resolution, const Coordinates &c)
{
	Q_UNUSED(c);

	if (_ozf.isOpen()) {
		for (int i = 0; i < _ozf.zooms(); i++) {
			rescale(i);
			qreal sr = _resolution / ((_scale.x() + _scale.y()) / 2.0);
			if (sr >= resolution)
				break;
		}
	}

	return _zoom;
}

qreal OfflineMap::zoomIn()
{
	if (_ozf.isOpen())
		rescale(qMax(_zoom - 1, 0));

	return _zoom;
}

qreal OfflineMap::zoomOut()
{
	if (_ozf.isOpen())
		rescale(qMin(_zoom + 1, _ozf.zooms() - 1));

	return _zoom;
}

void OfflineMap::rescale(int zoom)
{
	_zoom = zoom;
	_scale = QPointF(
	  (qreal)_ozf.size(_zoom).width() / (qreal)_ozf.size(0).width(),
	  (qreal)_ozf.size(_zoom).height() / (qreal)_ozf.size(0).height());
}
