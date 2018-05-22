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
#include "tar.h"
#include "ozf.h"
#include "mapfile.h"
#include "geotiff.h"
#include "offlinemap.h"


bool OfflineMap::setImageInfo(const QString &path)
{
	QFileInfo ii(_map.path);

	if (ii.isRelative())
		ii.setFile(path + "/" + _map.path);

	if (!ii.exists()) {
		int last = _map.path.lastIndexOf('\\');
		if (last >= 0 && last < _map.path.length() - 1) {
			QStringRef fn(&_map.path, last + 1, _map.path.length() - last - 1);
			ii.setFile(path + "/" + fn.toString());
		}
	}

	if (ii.exists())
		_map.path = ii.absoluteFilePath();
	else {
		_errorString = QString("%1: No such image file").arg(_map.path);
		return false;
	}

	if (OZF::isOZF(_map.path)) {
		_ozf = new OZF(_map.path);
		if (!_ozf->open()) {
			_errorString = QString("%1: Error loading OZF file")
			  .arg(_ozf->fileName());
			return false;
		}
		_scale = _ozf->scale(_zoom);
	} else {
		QImageReader img(_map.path);
		_map.size = img.size();
		if (!_map.size.isValid()) {
			_errorString = QString("%1: Error reading map image")
			  .arg(QFileInfo(_map.path).fileName());
			return false;
		}
	}

	return true;
}

bool OfflineMap::setTileInfo(const QStringList &tiles, const QString &path)
{
	if (!_map.size.isValid()) {
		_errorString = "Missing total image size (IWH)";
		return false;
	}

	QRegExp rx("_[0-9]+_[0-9]+\\.");
	for (int i = 0; i < tiles.size(); i++) {
		if (tiles.at(i).contains(rx)) {
			_tile.path = QString(tiles.at(i)).replace(rx, "_%1_%2.");

			if (_tar) {
				QByteArray ba = _tar->file(tiles.at(i));
				QBuffer buffer(&ba);
				_tile.size = QImageReader(&buffer).size();
			} else {
				_tile.path = path + "/" + _tile.path;
				_tile.size = QImageReader(path + "/" + tiles.at(i)).size();
			}
			if (!_tile.size.isValid()) {
				_errorString = QString("Error retrieving tile size: "
				  "%1: Invalid image").arg(QFileInfo(tiles.at(i)).fileName());
				return false;
			}

			_map.path = QString();
			return true;
		}
	}

	_errorString = "Invalid/missing tile set";
	return false;
}

OfflineMap::OfflineMap(const QString &fileName, QObject *parent)
  : Map(parent), _img(0), _tar(0), _ozf(0), _zoom(0), _valid(false)
{
	QFileInfo fi(fileName);
	QString suffix = fi.suffix().toLower();


	if (suffix == "tar") {
		_tar = new Tar(fileName);
		if (!_tar->open()) {
			_errorString = "Error reading tar file";
			return;
		}

		QString mapFileName = fi.completeBaseName() + ".map";
		QByteArray ba = _tar->file(mapFileName);
		if (ba.isNull()) {
			_errorString = "Map file not found";
			return;
		}
		QBuffer buffer(&ba);
		MapFile mf(buffer);
		if (!mf.isValid()) {
			_errorString = mf.errorString();
			return;
		} else {
			_name = mf.name();
			_map.size = mf.size();
			_map.path = mf.image();
			_projection = mf.projection();
			_transform = mf.transform();
		}
	} else if (suffix == "map") {
		QFile file(fileName);
		MapFile mf(file);
		if (!mf.isValid()) {
			_errorString = mf.errorString();
			return;
		} else {
			_name = mf.name();
			_map.size = mf.size();
			_map.path = mf.image();
			_projection = mf.projection();
			_transform = mf.transform();
		}
	} else if (suffix == "tif" || suffix == "tiff") {
		GeoTIFF gt(fileName);
		if (!gt.isValid()) {
			_errorString = gt.errorString();
			return;
		} else {
			_name = fi.fileName();
			_map.path = fileName;
			_projection = gt.projection();
			_transform = gt.transform();
		}
	} else {
		_errorString = "Not a map file";
		return;
	}

	if (_tar) {
		if (!setTileInfo(_tar->files()))
			return;
	} else {
		QDir set(fi.absolutePath() + "/" + "set");
		if (set.exists()) {
			if (!setTileInfo(set.entryList(), set.absolutePath()))
				return;
		} else {
			if (!setImageInfo(fi.absolutePath()))
				return;
		}
	}

	_valid = true;
}

OfflineMap::OfflineMap(const QString &fileName, Tar &tar, QObject *parent)
  : Map(parent), _img(0), _tar(0), _ozf(0), _zoom(0), _valid(false)
{
	QFileInfo fi(fileName);
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
	MapFile mf(buffer);
	if (!mf.isValid()) {
		_errorString = mf.errorString();
		return;
	}

	_name = mf.name();
	_map.size = mf.size();
	_projection = mf.projection();
	_transform = mf.transform();
	_tar = new Tar(fi.absolutePath() + "/" + fi.completeBaseName() + ".tar");

	_valid = true;
}

OfflineMap::~OfflineMap()
{
	delete _img;
	delete _tar;
	delete _ozf;
}

void OfflineMap::load()
{
	if (_tar && !_tar->isOpen()) {
		if (!_tar->open()) {
			qWarning("%s: error loading tar file",
			  qPrintable(_tar->fileName()));
			return;
		}
		if (!setTileInfo(_tar->files()))
			qWarning("%s: %s", qPrintable(_tar->fileName()),
			  qPrintable(_errorString));
		return;
	}

	if (!_ozf && !_img && _map.isValid()) {
		_img = new QImage(_map.path);
		if (_img->isNull())
			qWarning("%s: error loading map image", qPrintable(_map.path));
	}
}

void OfflineMap::unload()
{
	delete _img;
	_img = 0;
}

void OfflineMap::drawTiled(QPainter *painter, const QRectF &rect) const
{
	QPoint tl = QPoint((int)floor(rect.left() / (qreal)_tile.size.width())
	  * _tile.size.width(), (int)floor(rect.top() / _tile.size.height())
	  * _tile.size.height());

	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	for (int i = 0; i < ceil(s.width() / _tile.size.width()); i++) {
		for (int j = 0; j < ceil(s.height() / _tile.size.height()); j++) {
			int x = tl.x() + i * _tile.size.width();
			int y = tl.y() + j * _tile.size.height();

			QString tileName(_tile.path.arg(QString::number(x),
			  QString::number(y)));
			QPixmap pixmap;

			if (_tar) {
				QString key = _tar->fileName() + "/" + tileName;
				if (!QPixmapCache::find(key, &pixmap)) {
					QByteArray ba = _tar->file(tileName);
					pixmap = QPixmap::fromImage(QImage::fromData(ba));
					if (!pixmap.isNull())
						QPixmapCache::insert(key, pixmap);
				}
			} else
				pixmap = QPixmap(tileName);

			if (pixmap.isNull())
				qWarning("%s: error loading tile image", qPrintable(
				  _tile.path.arg(QString::number(x), QString::number(y))));
			else
				painter->drawPixmap(QPoint(x, y), pixmap);
		}
	}
}

void OfflineMap::drawOZF(QPainter *painter, const QRectF &rect) const
{
	QPoint tl = QPoint((int)floor(rect.left() / _ozf->tileSize().width())
	  * _ozf->tileSize().width(), (int)floor(rect.top()
	  / _ozf->tileSize().height()) * _ozf->tileSize().height());

	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	for (int i = 0; i < ceil(s.width() / _ozf->tileSize().width()); i++) {
		for (int j = 0; j < ceil(s.height() / _ozf->tileSize().height()); j++) {
			int x = tl.x() + i * _ozf->tileSize().width();
			int y = tl.y() + j * _ozf->tileSize().height();

			QPixmap pixmap;
			QString key = _ozf->fileName() + "/" + QString::number(_zoom) + "_"
			  + QString::number(x) + "_" + QString::number(y);
			if (!QPixmapCache::find(key, &pixmap)) {
				pixmap = _ozf->tile(_zoom, x, y);
				if (!pixmap.isNull())
					QPixmapCache::insert(key, pixmap);
			}

			if (pixmap.isNull())
				qWarning("%s: error loading tile image", qPrintable(key));
			else
				painter->drawPixmap(QPoint(x, y), pixmap);
		}
	}
}

void OfflineMap::drawImage(QPainter *painter, const QRectF &rect) const
{
	QRect r(rect.toRect());
	painter->drawImage(r.left(), r.top(), *_img, r.left(), r.top(),
	  r.width(), r.height());
}

void OfflineMap::draw(QPainter *painter, const QRectF &rect, bool block)
{
	Q_UNUSED(block);

	if (_ozf)
		drawOZF(painter, rect);
	else if (_tile.isValid())
		drawTiled(painter, rect);
	else if (_img && !_img->isNull())
		drawImage(painter, rect);
}

QPointF OfflineMap::ll2xy(const Coordinates &c) const
{
	QPointF p(_transform.proj2img(_projection.ll2xy(c)));
	return _ozf ? QPointF(p.x() * _scale.x(), p.y() * _scale.y()) : p;
}

Coordinates OfflineMap::xy2ll(const QPointF &p) const
{
	return _ozf
	  ? _projection.xy2ll(_transform.img2proj(QPointF(p.x() / _scale.x(),
		p.y() / _scale.y())))
	  : _projection.xy2ll(_transform.img2proj(p));
}

QRectF OfflineMap::bounds() const
{
	return _ozf
	  ? QRectF(QPointF(0, 0), _ozf->size(_zoom))
	  : QRectF(QPointF(0, 0), _map.size);
}

qreal OfflineMap::resolution(const QRectF &rect) const
{
	Coordinates tl = xy2ll((rect.topLeft()));
	Coordinates br = xy2ll(rect.bottomRight());

	qreal ds = tl.distanceTo(br);
	qreal ps = QLineF(rect.topLeft(), rect.bottomRight()).length();

	return ds/ps;
}

int OfflineMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (!_ozf)
		return _zoom;

	if (!rect.isValid())
		rescale(0);
	else {
		QPointF tl(_transform.proj2img(_projection.ll2xy(rect.topLeft())));
		QPointF br(_transform.proj2img(_projection.ll2xy(rect.bottomRight())));
		QRect sbr(QRectF(tl, br).toRect().normalized());

		for (int i = 0; i < _ozf->zooms(); i++) {
			rescale(i);
			if (sbr.size().width() * _scale.x() <= size.width()
			  && sbr.size().height() * _scale.y() <= size.height())
				break;
		}
	}

	return _zoom;
}

int OfflineMap::zoomIn()
{
	if (_ozf)
		rescale(qMax(_zoom - 1, 0));

	return _zoom;
}

int OfflineMap::zoomOut()
{
	if (_ozf)
		rescale(qMin(_zoom + 1, _ozf->zooms() - 1));

	return _zoom;
}

void OfflineMap::rescale(int zoom)
{
	_zoom = zoom;
	_scale = _ozf->scale(zoom);
}
