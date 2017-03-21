#include <QtGlobal>
#include <QPainter>
#include <QFileInfo>
#include <QMap>
#include <QDir>
#include <QBuffer>
#include <QImage>
#include <QImageReader>
#include <QPixmapCache>
#include "misc.h"
#include "rd.h"
#include "wgs84.h"
#include "coordinates.h"
#include "matrix.h"
#include "offlinemap.h"


int OfflineMap::parseMapFile(QIODevice &device, QList<ReferencePoint> &points)
{
	bool res;
	int ln = 1;


	if (!device.open(QIODevice::ReadOnly))
		return -1;

	while (!device.atEnd()) {
		QByteArray line = device.readLine();

		if (ln == 1) {
			if (line.trimmed() != "OziExplorer Map Data File Version 2.2")
				return ln;
		} else if (ln == 3)
			_imgPath = line.trimmed();
		else {
			QList<QByteArray> list = line.split(',');
			QString key(list.at(0).trimmed());

			if (key.startsWith("Point") && list.count() == 17
			  && !list.at(2).trimmed().isEmpty()) {
				int x = list.at(2).trimmed().toInt(&res);
				if (!res)
					return ln;
				int y = list.at(3).trimmed().toInt(&res);
				if (!res)
					return ln;
				int latd = list.at(6).trimmed().toInt(&res);
				if (!res)
					return ln;
				qreal latm = list.at(7).trimmed().toFloat(&res);
				if (!res)
					return ln;
				int lond = list.at(9).trimmed().toInt(&res);
				if (!res)
					return ln;
				qreal lonm = list.at(10).trimmed().toFloat(&res);
				if (!res)
					return ln;

				if (list.at(8).trimmed() == "S")
					latd = -latd;
				if (list.at(11).trimmed() == "W")
					lond = -lond;
				points.append(QPair<QPoint, Coordinates>(QPoint(x, y),
				  Coordinates(lond + lonm/60.0, latd + latm/60.0)));
			} else if (key == "IWH") {
				int w = list.at(2).trimmed().toInt(&res);
				if (!res)
					return ln;
				int h = list.at(3).trimmed().toInt(&res);
				if (!res)
					return ln;
				_size = QSize(w, h);
			}
		}

		ln++;
	}

	return 0;
}

bool OfflineMap::computeTransformation(const QList<ReferencePoint> &points)
{
	if (points.count() < 2)
		return false;

	Matrix c(3, 2);
	c.zeroize();
	for (size_t j = 0; j < c.w(); j++) {
		for (size_t k = 0; k < c.h(); k++) {
			for (int i = 0; i < points.size(); i++) {
				double f[3], t[2];
				QPointF p = points.at(i).second.toMercator();

				f[0] = p.x();
				f[1] = p.y();
				f[2] = 1.0;
				t[0] = points.at(i).first.x();
				t[1] = points.at(i).first.y();
				c.m(k,j) += f[k] * t[j];
			}
		}
	}

	Matrix Q(3, 3);
	Q.zeroize();
	for (int qi = 0; qi < points.size(); qi++) {
		double v[3];
		QPointF p = points.at(qi).second.toMercator();

		v[0] = p.x();
		v[1] = p.y();
		v[2] = 1.0;
		for (size_t i = 0; i < Q.h(); i++)
			for (size_t j = 0; j < Q.w(); j++)
				Q.m(i,j) += v[i] * v[j];
	}

	Matrix M = Q.augemented(c);
	if (!M.eliminate())
		return false;

	_transform = QTransform(M.m(0,3), M.m(1,3), M.m(0,4), M.m(1,4),
	  M.m(2,3), M.m(2,4));

	return true;
}

bool OfflineMap::computeResolution(QList<ReferencePoint> &points)
{
	if (points.count() < 2)
		return false;

	int maxLon = 0, minLon = 0, maxLat = 0, minLat = 0;
	qreal dLon, pLon, dLat, pLat;

	for (int i = 1; i < points.size(); i++) {
		if (points.at(i).second.lon() < points.at(minLon).second.lon())
			minLon = i;
		if (points.at(i).second.lon() > points.at(maxLon).second.lon())
			maxLon = i;
		if (points.at(i).second.lat() < points.at(minLat).second.lon())
			minLat = i;
		if (points.at(i).second.lat() > points.at(maxLat).second.lon())
			maxLat = i;
	}

	dLon = points.at(minLon).second.distanceTo(points.at(maxLon).second);
	pLon = QLineF(points.at(minLon).first, points.at(maxLon).first).length();
	dLat = points.at(minLat).second.distanceTo(points.at(maxLat).second);
	pLat = QLineF(points.at(minLat).first, points.at(maxLat).first).length();

	_resolution = (dLon/pLon + dLat/pLat) / 2.0;

	return true;
}

bool OfflineMap::getImageInfo(const QString &path)
{
	QFileInfo ii(_imgPath);
	if (ii.isRelative())
		_imgPath = path + "/" + _imgPath;

	QImageReader img(_imgPath);
	_size = img.size();
	if (!_size.isValid()) {
		qWarning("%s: %s: error reading map image", qPrintable(_name),
		  qPrintable(ii.absoluteFilePath()));
		return false;
	}

	return true;
}

bool OfflineMap::getTileInfo(const QStringList &tiles, const QString &path)
{
	if (!_size.isValid()) {
		qWarning("%s: missing total image size (IWH)", qPrintable(_name));
		return false;
	}

	if (tiles.isEmpty()) {
		qWarning("%s: empty tile set", qPrintable(_name));
		return false;
	}

	QRegExp rx("_[0-9]+_[0-9]+\\.");
	for (int i = 0; i < tiles.size(); i++) {
		if (tiles.at(i).contains(rx)) {
			_tileName = QString(tiles.at(i)).replace(rx, "_%1_%2.");

			if (_tar.isOpen()) {
				QByteArray ba = _tar.file(tiles.at(i));
				QBuffer buffer(&ba);
				_tileSize = QImageReader(&buffer).size();
			} else {
				_tileName = path + "/" + _tileName;
				_tileSize = QImageReader(path + "/" + tiles.at(i)).size();
			}
			if (!_tileSize.isValid()) {
				qWarning("%s: error retrieving tile size: %s: invalid image",
				  qPrintable(_name), qPrintable(QFileInfo(tiles.at(i))
				  .fileName()));
				return false;
			}

			return true;
		}
	}

	qWarning("%s: invalid tile names", qPrintable(_name));

	return false;
}

OfflineMap::OfflineMap(const QString &path, QObject *parent) : Map(parent)
{
	int errorLine = -2;
	QList<ReferencePoint> points;


	_valid = false;

	QFileInfo fi(path);
	_name = fi.fileName();

	QDir dir(path);
	QFileInfoList mapFiles = dir.entryInfoList(QDir::Files);
	for (int i = 0; i < mapFiles.count(); i++) {
		const QString &fileName = mapFiles.at(i).fileName();
		if (fileName.endsWith(".tar")) {
			if (!_tar.load(mapFiles.at(i).absoluteFilePath())) {
				qWarning("%s: %s: error loading tar file", qPrintable(_name),
				  qPrintable(fileName));
				return;
			}
			QStringList tarFiles = _tar.files();
			for (int j = 0; j < tarFiles.size(); j++) {
				if (tarFiles.at(j).endsWith(".map")) {
					QByteArray ba = _tar.file(tarFiles.at(j));
					QBuffer buffer(&ba);
					errorLine = parseMapFile(buffer, points);
					break;
				}
			}
			break;
		} else if (fileName.endsWith(".map")) {
			QFile mapFile(mapFiles.at(i).absoluteFilePath());
			errorLine = parseMapFile(mapFile, points);
			break;
		}
	}
	if (errorLine) {
		if (errorLine == -2)
			qWarning("%s: no map file found", qPrintable(_name));
		else if (errorLine == -1)
			qWarning("%s: error opening map file", qPrintable(_name));
		else
			qWarning("%s: map file parse error on line: %d", qPrintable(_name),
			  errorLine);
		return;
	}

	if (!computeTransformation(points)) {
		qWarning("%s: error computing map transformation", qPrintable(_name));
		return;
	}
	computeResolution(points);

	if (_tar.isOpen()) {
		if (!getTileInfo(_tar.files()))
			return;
	} else {
		QDir set(fi.absoluteFilePath() + "/" + "set");
		if (set.exists()) {
			if (!getTileInfo(set.entryList(), set.absolutePath()))
				return;
		} else {
			if (!getImageInfo(fi.absoluteFilePath()))
				return;
		}
	}

	_img = 0;
	_valid = true;
}

void OfflineMap::load()
{
	if (_tileSize.isValid())
		return;

	_img = new QImage(_imgPath);
	if (_img->isNull())
		qWarning("%s: error loading map image", qPrintable(_imgPath));
}

void OfflineMap::unload()
{
	if (_img)
		delete _img;
}

QRectF OfflineMap::bounds() const
{
	return QRectF(QPointF(0, 0), _size);
}

qreal OfflineMap::zoomFit(const QSize &size, const QRectF &br)
{
	Q_UNUSED(size);
	Q_UNUSED(br);

	return 1.0;
}

qreal OfflineMap::resolution(const QPointF &p) const
{
	Q_UNUSED(p);

	return _resolution;
}

qreal OfflineMap::zoomIn()
{
	return 1.0;
}

qreal OfflineMap::zoomOut()
{
	return 1.0;
}

void OfflineMap::draw(QPainter *painter, const QRectF &rect)
{
	if (_tileSize.isValid()) {
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
						QPixmapCache::insert(key, pixmap);
					}
				} else
					pixmap = QPixmap(tileName);

				if (pixmap.isNull()) {
					qWarning("%s: error loading tile image", qPrintable(
					  _tileName.arg(QString::number(x), QString::number(y))));
					painter->fillRect(QRectF(QPoint(x, y), _tileSize),
					  Qt::white);
				} else
					painter->drawPixmap(QPoint(x, y), pixmap);
			}
		}
	} else {
		if (_img->isNull())
			painter->fillRect(rect, Qt::white);
		else {
			QPoint p = rect.topLeft().toPoint();
			QImage crop = _img->copy(QRect(p, rect.size().toSize()));
			painter->drawImage(rect.topLeft(), crop);
		}
	}
}

QPointF OfflineMap::ll2xy(const Coordinates &c) const
{
	return _transform.map(c.toMercator());
}

Coordinates OfflineMap::xy2ll(const QPointF &p) const
{
	return Coordinates::fromMercator(_transform.inverted().map(p));
}
