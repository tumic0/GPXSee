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
#include "ellipsoid.h"
#include "latlon.h"
#include "mercator.h"
#include "transversemercator.h"
#include "utm.h"
#include "lambertconic.h"
#include "offlinemap.h"


int OfflineMap::parseMapFile(QIODevice &device, QList<ReferencePoint> &points,
  QString &projection, ProjectionSetup &setup, QString &datum)
{
	bool res;
	int ln = 1;


	if (!device.open(QIODevice::ReadOnly))
		return -1;

	while (!device.atEnd()) {
		QByteArray line = device.readLine();

		if (ln == 1) {
			if (!line.trimmed().startsWith("OziExplorer Map Data File"))
				return ln;
		} else if (ln == 3)
			_imgPath = line.trimmed();
		else if (ln == 5)
			datum = line.split(',').at(0).trimmed();
		else {
			QList<QByteArray> list = line.split(',');
			QString key(list.at(0).trimmed());
			bool ll = true; bool pp = true;

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
					ll = false;
				qreal latm = list.at(7).trimmed().toFloat(&res);
				if (!res)
					ll = false;
				int lond = list.at(9).trimmed().toInt(&res);
				if (!res)
					ll = false;
				qreal lonm = list.at(10).trimmed().toFloat(&res);
				if (!res)
					ll = false;
				if (ll && list.at(8).trimmed() == "S")
					latd = -latd;
				if (ll && list.at(11).trimmed() == "W")
					lond = -lond;

				int zone = list.at(13).trimmed().toInt(&res);
				if (!res)
					zone = 0;
				qreal ppx = list.at(14).trimmed().toFloat(&res);
				if (!res)
					pp = false;
				qreal ppy = list.at(15).trimmed().toFloat(&res);
				if (!res)
					pp = false;
				if (list.at(16).trimmed() == "S")
					zone = -zone;

				ReferencePoint p;
				p.xy = QPoint(x, y);
				if (ll) {
					p.ll = Coordinates(lond + lonm/60.0, latd + latm/60.0);
					points.append(p);
				} else if (pp) {
					p.pp = QPointF(ppx, ppy);
					setup.zone = zone;
					points.append(p);
				} else
					return ln;
			} else if (key == "IWH") {
				int w = list.at(2).trimmed().toInt(&res);
				if (!res)
					return ln;
				int h = list.at(3).trimmed().toInt(&res);
				if (!res)
					return ln;
				_size = QSize(w, h);
			} else if (key == "Map Projection") {
				projection = list.at(1);
			} else if (key == "Projection Setup") {
				if (list.count() < 8)
					return ln;
				setup.centralParallel = list.at(1).trimmed().toFloat(&res);
				if (!res)
					setup.centralParallel = 0;
				setup.centralMeridian = list.at(2).trimmed().toFloat(&res);
				if (!res)
					setup.centralMeridian = 0;
				setup.scale = list.at(3).trimmed().toFloat(&res);
				if (!res)
					setup.scale = 1.0;
				setup.falseEasting = list.at(4).trimmed().toFloat(&res);
				if (!res)
					setup.falseEasting = 0;
				setup.falseNorthing = list.at(5).trimmed().toFloat(&res);
				if (!res)
					setup.falseNorthing = 0;
				setup.standardParallel1 = list.at(6).trimmed().toFloat(&res);
				if (!res)
					setup.standardParallel1 = 0;
				setup.standardParallel2 = list.at(7).trimmed().toFloat(&res);
				if (!res)
					setup.standardParallel2 = 0;
			}
		}

		ln++;
	}

	return 0;
}

bool OfflineMap::createProjection(const QString &datum,
  const QString &projection, const ProjectionSetup &setup,
  QList<ReferencePoint> &points)
{
	if (points.count() < 2) {
		qWarning("%s: insufficient number of reference points",
		  qPrintable(_name));
		return false;
	}

	if (projection == "Mercator")
		_projection = new Mercator();
	else if (projection == "Transverse Mercator")
		_projection = new TransverseMercator(Ellipsoid(datum),
		  setup.centralMeridian, setup.scale, setup.falseEasting,
		  setup.falseNorthing);
	else if (projection == "Latitude/Longitude")
		_projection = new LatLon();
	else if (projection == "Lambert Conformal Conic")
		_projection = new LambertConic(Ellipsoid(datum), setup.standardParallel1,
		  setup.standardParallel2, setup.centralParallel, setup.centralMeridian,
		  setup.scale, setup.falseEasting, setup.falseNorthing);
	else if (projection == "(UTM) Universal Transverse Mercator") {
		if (setup.zone)
			_projection = new UTM(setup.zone);
		else if (!points.first().ll.isNull())
			_projection = new UTM(points.first().ll);
		else {
			qWarning("%s: Can not determine UTM zone", qPrintable(_name));
			return false;
		}
	} else {
		qWarning("%s: %s: unsupported map projection", qPrintable(_name),
		  qPrintable(projection));
		return false;
	}

	for (int i = 0; i < points.size(); i++)
		if (points.at(i).ll.isNull())
			points[i].ll = _projection->xy2ll(points.at(i).pp);

	return true;
}

bool OfflineMap::computeTransformation(const QList<ReferencePoint> &points)
{
	Q_ASSERT(points.count() >= 2);

	Matrix c(3, 2);
	c.zeroize();
	for (size_t j = 0; j < c.w(); j++) {
		for (size_t k = 0; k < c.h(); k++) {
			for (int i = 0; i < points.size(); i++) {
				double f[3], t[2];
				QPointF p = _projection->ll2xy(points.at(i).ll);

				f[0] = p.x();
				f[1] = p.y();
				f[2] = 1.0;
				t[0] = points.at(i).xy.x();
				t[1] = points.at(i).xy.y();
				c.m(k,j) += f[k] * t[j];
			}
		}
	}

	Matrix Q(3, 3);
	Q.zeroize();
	for (int qi = 0; qi < points.size(); qi++) {
		double v[3];
		QPointF p = _projection->ll2xy(points.at(qi).ll);

		v[0] = p.x();
		v[1] = p.y();
		v[2] = 1.0;
		for (size_t i = 0; i < Q.h(); i++)
			for (size_t j = 0; j < Q.w(); j++)
				Q.m(i,j) += v[i] * v[j];
	}

	Matrix M = Q.augemented(c);
	if (!M.eliminate()) {
		qWarning("%s: singular transformation matrix", qPrintable(_name));
		return false;
	}

	_transform = QTransform(M.m(0,3), M.m(0,4), M.m(1,3), M.m(1,4), M.m(2,3),
	  M.m(2,4));

	return true;
}

bool OfflineMap::computeResolution(QList<ReferencePoint> &points)
{
	Q_ASSERT(points.count() >= 2);

	int maxLon = 0, minLon = 0, maxLat = 0, minLat = 0;
	qreal dLon, pLon, dLat, pLat;

	for (int i = 1; i < points.size(); i++) {
		if (points.at(i).ll.lon() < points.at(minLon).ll.lon())
			minLon = i;
		if (points.at(i).ll.lon() > points.at(maxLon).ll.lon())
			maxLon = i;
		if (points.at(i).ll.lat() < points.at(minLat).ll.lon())
			minLat = i;
		if (points.at(i).ll.lat() > points.at(maxLat).ll.lon())
			maxLat = i;
	}

	dLon = points.at(minLon).ll.distanceTo(points.at(maxLon).ll);
	pLon = QLineF(points.at(minLon).xy, points.at(maxLon).xy).length();
	dLat = points.at(minLat).ll.distanceTo(points.at(maxLat).ll);
	pLat = QLineF(points.at(minLat).xy, points.at(maxLat).xy).length();

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
	if (tiles.isEmpty()) {
		qWarning("%s: empty tile set", qPrintable(_name));
		return false;
	}

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

bool OfflineMap::mapLoaded(int res)
{
	if (res) {
		if (res == -2)
			qWarning("%s: no map file found", qPrintable(_name));
		else if (res == -1)
			qWarning("%s: error opening map file", qPrintable(_name));
		else
			qWarning("%s: map file parse error on line: %d", qPrintable(_name),
			  res);
		return false;
	}

	return true;
}

bool OfflineMap::totalSizeSet()
{
	if (!_size.isValid()) {
		qWarning("%s: missing total image size (IWH)", qPrintable(_name));
		return false;
	} else
		return true;
}

OfflineMap::OfflineMap(const QString &path, QObject *parent) : Map(parent)
{
	int errorLine = -2;
	QList<ReferencePoint> points;
	QString proj, datum;
	ProjectionSetup setup;


	_valid = false;
	_img = 0;
	_projection = 0;
	_resolution = 0;

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
					errorLine = parseMapFile(buffer, points, proj, setup, datum);
					_imgPath = QString();
					break;
				}
			}
			break;
		} else if (fileName.endsWith(".map")) {
			QFile mapFile(mapFiles.at(i).absoluteFilePath());
			errorLine = parseMapFile(mapFile, points, proj, setup, datum);
			break;
		}
	}
	if (!mapLoaded(errorLine))
		return;

	if (!createProjection(datum, proj, setup, points))
		return;
	if (!computeTransformation(points))
		return;
	computeResolution(points);

	if (_tar.isOpen()) {
		if (!totalSizeSet())
			return;
		if (!getTileInfo(_tar.files()))
			return;
	} else {
		QDir set(fi.absoluteFilePath() + "/" + "set");
		if (set.exists()) {
			if (!totalSizeSet())
				return;
			if (!getTileInfo(set.entryList(), set.canonicalPath()))
				return;
			_imgPath = QString();
		} else {
			if (!getImageInfo(fi.absoluteFilePath()))
				return;
		}
	}

	_valid = true;
}

OfflineMap::OfflineMap(Tar &tar, const QString &path, QObject *parent)
  : Map(parent)
{
	int errorLine = -2;
	QList<ReferencePoint> points;
	QString proj, datum;
	ProjectionSetup setup;


	_valid = false;
	_img = 0;
	_projection = 0;

	QFileInfo fi(path);
	_name = fi.fileName();

	QFileInfo li(fi.absoluteDir().dirName());
	QString prefix = li.fileName() + "/" + fi.fileName() + "/";
	QStringList tarFiles = tar.files();
	for (int j = 0; j < tarFiles.size(); j++) {
		if (tarFiles.at(j).startsWith(prefix)) {
			QByteArray ba = tar.file(tarFiles.at(j));
			QBuffer buffer(&ba);
			errorLine = parseMapFile(buffer, points, proj, setup, datum);
			break;
		}
	}
	if (!mapLoaded(errorLine))
		return;

	if (!createProjection(datum, proj, setup, points))
		return;
	if (!totalSizeSet())
		return;
	if (!computeTransformation(points))
		return;
	computeResolution(points);

	QDir dir(path);
	QFileInfoList mapFiles = dir.entryInfoList(QDir::Files);
	for (int i = 0; i < mapFiles.count(); i++) {
		const QString &fileName = mapFiles.at(i).absoluteFilePath();
		if (fileName.endsWith(".tar"))
			_tarPath = fileName;
	}

	_imgPath = QString();
	_valid = true;
}

OfflineMap::~OfflineMap()
{
	if (_img)
		delete _img;
	if (_projection)
		delete _projection;
}

void OfflineMap::load()
{
	if (!_tarPath.isNull() && !_tileSize.isValid()) {
		if (!_tar.load(_tarPath)) {
			qWarning("%s: %s: error loading tar file", qPrintable(_name),
			  qPrintable(_tarPath));
			return;
		}
		getTileInfo(_tar.files());
		return;
	}

	if (!_img && !_imgPath.isNull()) {
		_img = new QImage(_imgPath);
		if (_img->isNull())
			qWarning("%s: error loading map image", qPrintable(_imgPath));
	}
}

void OfflineMap::unload()
{
	if (_img) {
		delete _img;
		_img = 0;
	}
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
						if (!pixmap.isNull())
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
		if (!_img || _img->isNull())
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
	return _transform.map(_projection->ll2xy(c));
}

Coordinates OfflineMap::xy2ll(const QPointF &p) const
{
	return _projection->xy2ll(_transform.inverted().map(p));
}

QPointF OfflineMap::ll2pp(const Coordinates &c) const
{
	return _projection->ll2xy(c);
}

QPointF OfflineMap::xy2pp(const QPointF &p) const
{
	return _transform.inverted().map(p);
}

QPointF OfflineMap::pp2xy(const QPointF &p) const
{
	return _transform.map(p);
}
