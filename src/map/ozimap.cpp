#include <QPainter>
#include <QFileInfo>
#include <QMap>
#include <QDir>
#include <QBuffer>
#include <QImageReader>
#include <QPixmapCache>
#include <QRegularExpression>
#include "common/coordinates.h"
#include "common/rectc.h"
#include "tar.h"
#include "ozf.h"
#include "image.h"
#include "mapfile.h"
#include "gmifile.h"
#include "rectd.h"
#include "ozimap.h"


static QString tarFile(const QString &path)
{
	QDir dir(path);
	QFileInfoList files = dir.entryInfoList(QDir::Files);

	for (int i = 0; i < files.size(); i++) {
		const QFileInfo &fi = files.at(i);

		if (fi.suffix().toLower() == "tar")
			return fi.absoluteFilePath();
	}

	return QString();
}

QString OziMap::calibrationFile(const QStringList &files, const QString path,
  CalibrationType &type)
{
	for (int i = 0; i < files.size(); i++) {
		QFileInfo fi(files.at(i));
		QString suffix(fi.suffix().toLower());

		if (path.endsWith(fi.path())) {
			if (suffix == "map") {
				type = MAP;
				return files.at(i);
			} else if (suffix == "gmi") {
				type = GMI;
				return files.at(i);
			}
		}
	}

	type = Unknown;
	return QString();
}

OziMap::OziMap(const QString &fileName, CalibrationType type,
  const Projection &proj, QObject *parent) : Map(fileName, parent), _img(0),
  _tar(0), _ozf(0), _zoom(0), _mapRatio(1.0), _valid(false)
{
	// TAR maps
	if (type == Unknown) {
		_tar = new Tar(fileName);
		if (!_tar->open()) {
			_errorString = _tar->errorString();
			return;
		}
		QStringList files(_tar->files());
		QString cf(calibrationFile(files, ".", type));

		if (type == GMI) {
			QByteArray ba(_tar->file(cf));
			QBuffer buffer(&ba);
			GmiFile gmi(buffer);
			if (!gmi.isValid()) {
				_errorString = gmi.errorString();
				return;
			} else {
				_name = Util::file2name(fileName);
				_map.size = gmi.size();
				_map.path = gmi.image();
				_calibrationPoints = gmi.calibrationPoints();
				_projection = proj;
				computeTransform();
			}
		} else if (type == MAP) {
			QByteArray ba(_tar->file(cf));
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
		} else {
			_errorString = "No calibration file found";
			return;
		}

		if (!setTileInfo(files))
			return;

		_tar->close();

	// regular MAP or GMI maps
	} else {
		QFile file(fileName);

		if (type == MAP) {
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
		} else if (type == GMI) {
			GmiFile gmi(file);
			if (!gmi.isValid()) {
				_errorString = gmi.errorString();
				return;
			} else {
				_name = Util::file2name(fileName);
				_map.size = gmi.size();
				_map.path = gmi.image();
				_calibrationPoints = gmi.calibrationPoints();
				_projection = proj;
				computeTransform();
			}
		}

		QFileInfo fi(fileName);
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

OziMap::OziMap(const QString &dirName, Tar &tar, const Projection &proj,
  QObject *parent) : Map(dirName, parent), _img(0), _tar(0), _ozf(0), _zoom(0),
  _mapRatio(1.0), _valid(false)
{
	CalibrationType type;
	QString cf(calibrationFile(tar.files(), dirName, type));

	if (type == MAP) {
		QByteArray ba = tar.file(cf);
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
	} else if (type == GMI) {
		QByteArray ba = tar.file(cf);
		QBuffer buffer(&ba);
		GmiFile gmi(buffer);
		if (!gmi.isValid()) {
			_errorString = gmi.errorString();
			return;
		}

		_name = Util::file2name(cf);
		_map.size = gmi.size();
		_calibrationPoints = gmi.calibrationPoints();
		_projection = proj;
		computeTransform();
	} else {
		_errorString = "No calibration file found";
		return;
	}

	QString tf(tarFile(dirName));
	if (tf.isNull()) {
		_errorString = "No map tar file found";
		return;
	}
	_tar = new Tar(tf);
	if (!_tar->open()) {
		_errorString = _tar->fileName() + ": " + _tar->errorString();
		return;
	}
	if (!setTileInfo(_tar->files())) {
		_errorString = _tar->fileName() + ": " + _errorString;
		return;
	}
	_tar->close();

	_valid = true;
}

OziMap::~OziMap()
{
	delete _img;
	delete _tar;
	delete _ozf;
}

bool OziMap::setImageInfo(const QString &path)
{
	QFileInfo ii(_map.path);

	if (ii.isRelative())
		ii.setFile(path + "/" + _map.path);

	if (!ii.exists()) {
		int last = _map.path.lastIndexOf('\\');
		if (last >= 0 && last < _map.path.length() - 1) {
			QString fn(_map.path.mid(last + 1, _map.path.length() - last - 1));
			ii.setFile(path + "/" + fn);
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
			_errorString = QString("%1: %2").arg(_map.path, _ozf->errorString());
			return false;
		}
		_scale = _ozf->scale(_zoom);
		_ozf->close();
	} else {
		QImageReader ir(_map.path);
		if (!ir.canRead()) {
			_errorString = QString("%1: Unsupported/invalid image file")
			  .arg(_map.path);
			return false;
		}
		_map.size = ir.size();
	}

	return true;
}

bool OziMap::setTileInfo(const QStringList &tiles, const QString &path)
{
	static QRegularExpression rx("_[0-9]+_[0-9]+\\.");

	if (!_map.size.isValid()) {
		_errorString = "Missing total image size (IWH)";
		return false;
	}

	for (int i = 0; i < tiles.size(); i++) {
		const QString &tile = tiles.at(i);

		if (tile.startsWith("set/") && tile.contains(rx)) {
			_tile.path = QString(tile).replace(rx, "_%1_%2.");

			if (_tar) {
				QByteArray ba(_tar->file(tile));
				QBuffer buffer(&ba);
				_tile.size = QImageReader(&buffer).size();
			} else {
				_tile.path = path + "/" + _tile.path;
				_tile.size = QImageReader(path + "/" + tile).size();
			}
			if (!_tile.size.isValid()) {
				_errorString = QString("Error retrieving tile size: "
				  "%1: Invalid image").arg(QFileInfo(tile).fileName());
				return false;
			}

			_map.path = QString();
			return true;
		}
	}

	_errorString = "Invalid/missing tile set";
	return false;
}

void OziMap::load(const Projection &in, const Projection &out,
  qreal deviceRatio, bool hidpi)
{
	Q_UNUSED(out);

	_mapRatio = hidpi ? deviceRatio : 1.0;
	if (!_calibrationPoints.isEmpty()) {
		_projection = in;
		computeTransform();
	}

	if (_tar) {
		Q_ASSERT(!_tar->isOpen());
		if (!_tar->open()) {
			qWarning("%s: %s", qPrintable(_tar->fileName()),
			  qPrintable(_tar->errorString()));
			return;
		}
	}
	if (_ozf) {
		Q_ASSERT(!_ozf->isOpen());
		if (!_ozf->open()) {
			qWarning("%s: %s", qPrintable(_ozf->fileName()),
			  qPrintable(_ozf->errorString()));
			return;
		}
	}
	if (!_tile.isValid() && !_ozf) {
		Q_ASSERT(!_img);
		_img = new Image(_map.path);
		_img->setDevicePixelRatio(_mapRatio);
	}
}

void OziMap::unload()
{
	delete _img;
	_img = 0;

	if (_tar && _tar->isOpen())
		_tar->close();

	if (_ozf && _ozf->isOpen())
		_ozf->close();
}

void OziMap::drawTiled(QPainter *painter, const QRectF &rect) const
{
	QSizeF ts(_tile.size.width() / _mapRatio, _tile.size.height() / _mapRatio);
	QPointF tl(floor(rect.left() / ts.width()) * ts.width(),
	  floor(rect.top() / ts.height()) * ts.height());

	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	for (int i = 0; i < ceil(s.width() / ts.width()); i++) {
		for (int j = 0; j < ceil(s.height() / ts.height()); j++) {
			int x = round(tl.x() * _mapRatio + i * _tile.size.width());
			int y = round(tl.y() * _mapRatio + j * _tile.size.height());

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
				qWarning("%s: error loading tile image", qPrintable(tileName));
			else {
				pixmap.setDevicePixelRatio(_mapRatio);
				QPointF tp(tl.x() + i * ts.width(), tl.y() + j * ts.height());
				painter->drawPixmap(tp, pixmap);
			}
		}
	}
}

void OziMap::drawOZF(QPainter *painter, const QRectF &rect) const
{
	QSizeF ts(_ozf->tileSize().width() / _mapRatio, _ozf->tileSize().height()
	  / _mapRatio);
	QPointF tl(floor(rect.left() / ts.width()) * ts.width(),
	  floor(rect.top() / ts.height()) * ts.height());

	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	for (int i = 0; i < ceil(s.width() / ts.width()); i++) {
		for (int j = 0; j < ceil(s.height() / ts.height()); j++) {
			int x = round(tl.x() * _mapRatio + i * _ozf->tileSize().width());
			int y = round(tl.y() * _mapRatio + j * _ozf->tileSize().height());

			QPixmap pixmap;
			QString key(_ozf->fileName() + "/" + QString::number(_zoom) + "_"
			  + QString::number(x) + "_" + QString::number(y));

			if (!QPixmapCache::find(key, &pixmap)) {
				pixmap = _ozf->tile(_zoom, x, y);
				if (!pixmap.isNull())
					QPixmapCache::insert(key, pixmap);
			}

			if (pixmap.isNull())
				qWarning("%s: error loading tile image", qPrintable(key));
			else {
				pixmap.setDevicePixelRatio(_mapRatio);
				QPointF tp(tl.x() + i * ts.width(), tl.y() + j * ts.height());
				painter->drawPixmap(tp, pixmap);
			}
		}
	}
}

void OziMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	Q_UNUSED(flags);

	if (_ozf)
		drawOZF(painter, rect);
	else if (_img)
		_img->draw(painter, rect, flags);
	else if (_tile.isValid())
		drawTiled(painter, rect);
}

QPointF OziMap::ll2xy(const Coordinates &c)
{
	QPointF p(_transform.proj2img(_projection.ll2xy(c)));
	return _ozf
	  ? QPointF(p.x() * _scale.x(), p.y() * _scale.y()) / _mapRatio
	  : p / _mapRatio;
}

Coordinates OziMap::xy2ll(const QPointF &p)
{
	return _ozf
	  ? _projection.xy2ll(_transform.img2proj(QPointF(p.x() / _scale.x(),
		p.y() / _scale.y()) * _mapRatio))
	  : _projection.xy2ll(_transform.img2proj(p * _mapRatio));
}

QRectF OziMap::bounds()
{
	return _ozf
	  ? QRectF(QPointF(0, 0), _ozf->size(_zoom) / _mapRatio)
	  : QRectF(QPointF(0, 0), _map.size / _mapRatio);
}

int OziMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (!_ozf)
		return _zoom;

	if (!rect.isValid())
		rescale(0);
	else {
		RectD prect(rect, _projection);
		QRectF sbr(_transform.proj2img(prect.topLeft()),
		  _transform.proj2img(prect.bottomRight()));

		for (int i = 0; i < _ozf->zooms(); i++) {
			rescale(i);
			if (sbr.size().width() * _scale.x() <= size.width()
			  && sbr.size().height() * _scale.y() <= size.height())
				break;
		}
	}

	return _zoom;
}

int OziMap::zoomIn()
{
	if (_ozf)
		rescale(qMax(_zoom - 1, 0));

	return _zoom;
}

int OziMap::zoomOut()
{
	if (_ozf)
		rescale(qMin(_zoom + 1, _ozf->zooms() - 1));

	return _zoom;
}

void OziMap::rescale(int zoom)
{
	_zoom = zoom;
	_scale = _ozf->scale(zoom);
}

void OziMap::computeTransform()
{
	QList<ReferencePoint> rp;

	for (int i = 0; i < _calibrationPoints.size(); i++)
		rp.append(_calibrationPoints.at(i).rp(_projection));

	_transform = Transform(rp);
}

Map *OziMap::createTAR(const QString &path, const Projection &proj, bool *isDir)
{
	if (isDir)
		*isDir = false;

	return new OziMap(path, Unknown, proj);
}

Map *OziMap::createMAP(const QString &path, const Projection &proj, bool *isDir)
{
	if (isDir)
		*isDir = false;

	return new OziMap(path, MAP, proj);
}

Map *OziMap::createGMI(const QString &path, const Projection &proj, bool *isDir)
{
	if (isDir)
		*isDir = false;

	return new OziMap(path, GMI, proj);
}
