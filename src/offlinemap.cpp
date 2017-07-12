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
#include "datum.h"
#include "latlon.h"
#include "mercator.h"
#include "transversemercator.h"
#include "utm.h"
#include "lambertconic.h"
#include "albersequal.h"
#include "ozf.h"
#include "offlinemap.h"


int OfflineMap::parse(QIODevice &device, QList<ReferencePoint> &points,
  QString &projection, ProjectionSetup &setup, QString &datum)
{
	bool res;
	int ln = 1;

	while (!device.atEnd()) {
		QByteArray line = device.readLine();

		if (ln == 1) {
			if (!line.trimmed().startsWith("OziExplorer Map Data File"))
				return ln;
		} else if (ln == 2)
			_name = line.trimmed();
		else if (ln == 3)
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
				if (ll && list.at(8).trimmed() == "S") {
					latd = -latd;
					latm = -latm;
				}
				if (ll && list.at(11).trimmed() == "W") {
					lond = -lond;
					lonm = -lonm;
				}

				setup.zone = list.at(13).trimmed().toInt(&res);
				if (!res)
					setup.zone = 0;
				qreal ppx = list.at(14).trimmed().toFloat(&res);
				if (!res)
					pp = false;
				qreal ppy = list.at(15).trimmed().toFloat(&res);
				if (!res)
					pp = false;
				if (list.at(16).trimmed() == "S")
					setup.zone = -setup.zone;

				ReferencePoint p;
				p.xy = QPoint(x, y);
				if (ll) {
					p.ll = Coordinates(lond + lonm/60.0, latd + latm/60.0);
					if (p.ll.isValid())
						points.append(p);
					else
						return ln;
				} else if (pp) {
					p.pp = QPointF(ppx, ppy);
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
				setup.latitudeOrigin = list.at(1).trimmed().toFloat(&res);
				if (!res)
					setup.latitudeOrigin = 0;
				setup.longitudeOrigin = list.at(2).trimmed().toFloat(&res);
				if (!res)
					setup.longitudeOrigin = 0;
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

bool OfflineMap::parseMapFile(QIODevice &device, QList<ReferencePoint> &points,
  QString &projection, ProjectionSetup &setup, QString &datum)
{
	int el;

	if (!device.open(QIODevice::ReadOnly)) {
		_errorString = QString("Error opening map file: %1")
		  .arg(device.errorString());
		return false;
	}

	if ((el = parse(device, points, projection, setup, datum))) {
		_errorString = QString("Map file parse error on line %1").arg(el);
		return false;
	}

	return true;
}

bool OfflineMap::createProjection(const QString &datum,
  const QString &projection, const ProjectionSetup &setup,
  QList<ReferencePoint> &points)
{
	if (points.count() < 2) {
		_errorString = "Insufficient number of reference points";
		return false;
	}

	Datum d = Datum::datum(datum);
	if (d.isNull()) {
		_errorString = QString("%1: Unknown datum").arg(datum);
		return false;
	}

	if (setup.latitudeOrigin < -90.0 || setup.latitudeOrigin > 90.0
	  || setup.longitudeOrigin < -180.0 || setup.longitudeOrigin > 180.0
	  || setup.standardParallel1 < -90.0 || setup.standardParallel1 > 90.0
	  || setup.standardParallel2 < -90.0 || setup.standardParallel2 > 90.0) {
		_errorString = "Invalid projection setup";
		return false;
	}

	if (projection == "Mercator")
		_projection = new Mercator();
	else if (projection == "Transverse Mercator")
		_projection = new TransverseMercator(d.ellipsoid(),
		  setup.latitudeOrigin, setup.longitudeOrigin, setup.scale,
		  setup.falseEasting, setup.falseNorthing);
	else if (projection == "Latitude/Longitude")
		_projection = new LatLon();
	else if (projection == "Lambert Conformal Conic")
		_projection = new LambertConic(d.ellipsoid(),
		  setup.standardParallel1, setup.standardParallel2,
		  setup.latitudeOrigin, setup.longitudeOrigin, setup.scale,
		  setup.falseEasting, setup.falseNorthing);
	else if (projection == "Albers Equal Area")
		_projection = new AlbersEqual(d.ellipsoid(), setup.standardParallel1,
		  setup.standardParallel2, setup.latitudeOrigin, setup.longitudeOrigin,
		  setup.falseEasting, setup.falseNorthing);
	else if (projection == "(UTM) Universal Transverse Mercator") {
		if (setup.zone)
			_projection = new UTM(d.ellipsoid(), setup.zone);
		else if (!points.first().ll.isNull())
			_projection = new UTM(d.ellipsoid(), points.first().ll);
		else {
			_errorString = "Can not determine UTM zone";
			return false;
		}
	} else if (projection == "(NZTM2) New Zealand TM 2000")
		_projection = new TransverseMercator(d.ellipsoid(), 0, 173.0, 0.9996,
		  1600000, 10000000);
	else if (projection == "(BNG) British National Grid")
		_projection = new TransverseMercator(d.ellipsoid(), 49, -2, 0.999601,
		  400000, -100000);
	else if (projection == "(IG) Irish Grid")
		_projection = new TransverseMercator(d.ellipsoid(), 53.5, -8, 1.000035,
		  200000, 250000);
	else if (projection == "(SG) Swedish Grid")
		_projection = new TransverseMercator(d.ellipsoid(), 0, 15.808278, 1,
		  1500000, 0);
	else {
		_errorString = QString("%1: Unknown map projection").arg(projection);
		return false;
	}

	for (int i = 0; i < points.size(); i++) {
		if (points.at(i).ll.isNull())
			points[i].ll = d.toWGS84(_projection->xy2ll(points.at(i).pp));
		else
			points[i].ll = d.toWGS84(points[i].ll);
	}

	return true;
}

bool OfflineMap::computeTransformation(const QList<ReferencePoint> &points)
{
	Q_ASSERT(points.size() >= 2);

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
		_errorString = "Singular transformation matrix";
		return false;
	}

	_transform = QTransform(M.m(0,3), M.m(0,4), M.m(1,3), M.m(1,4), M.m(2,3),
	  M.m(2,4));
	_inverted = _transform.inverted();

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

	QString suffix = ii.suffix().toLower();
	if (suffix == "ozf4" || suffix == "ozfx4") {
		_errorString = QString("%1: OZF4 image files not supported")
		  .arg(QFileInfo(_imgPath).fileName());
		return false;
	} else if (suffix == "ozf2" || suffix == "ozfx2" || suffix == "ozf3"
	  || suffix == "ozfx3") {
		_ozf.load(_imgPath);
		_size = _ozf.size();
	} else {
		QImageReader img(_imgPath);
		_size = img.size();
	}
	if (!_size.isValid()) {
		_errorString = QString("%1: Error reading map image")
		  .arg(QFileInfo(_imgPath).fileName());
		return false;
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
	QList<ReferencePoint> points;
	QString proj, datum;
	ProjectionSetup setup;
	QFileInfo fi(fileName);
	QString suffix = fi.suffix().toLower();


	_valid = false;
	_img = 0;
	_projection = 0;
	_resolution = 0.0;

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
		if (!parseMapFile(mapFile, points, proj, setup, datum))
			return;
	} else if (suffix =="map") {
		QFile mapFile(fileName);
		if (!parseMapFile(mapFile, points, proj, setup, datum))
			return;
	} else {
		_errorString = "Not a map file";
		return;
	}

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

	_valid = true;
}

OfflineMap::OfflineMap(const QString &fileName, Tar &tar, QObject *parent)
  : Map(parent)
{
	QList<ReferencePoint> points;
	QString proj, datum;
	ProjectionSetup setup;
	QFileInfo fi(fileName);

	_valid = false;
	_img = 0;
	_projection = 0;
	_resolution = 0.0;

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

	if (!parseMapFile(buffer, points, proj, setup, datum))
		return;
	if (!createProjection(datum, proj, setup, points))
		return;
	if (!totalSizeSet())
		return;
	if (!computeTransformation(points))
		return;
	computeResolution(points);


	_tarPath = fi.absolutePath() + "/" + fi.completeBaseName() + ".tar";
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
	if (_img) {
		delete _img;
		_img = 0;
	}
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

			if (!QRectF(QPointF(x, y), _ozf.tileSize()).intersects(bounds())) {
				painter->fillRect(QRectF(QPoint(x, y), _tileSize), Qt::white);
				continue;
			}

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
				painter->fillRect(QRectF(QPoint(x, y), _tileSize), Qt::white);
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

			if (!QRectF(QPointF(x, y), _ozf.tileSize()).intersects(bounds())) {
				painter->fillRect(QRectF(QPoint(x, y), _tileSize), Qt::white);
				continue;
			}

			QPixmap pixmap;
			QString key = _ozf.fileName() + "/" + QString::number(x)
			  + "_" + QString::number(y);
			if (!QPixmapCache::find(key, &pixmap)) {
				pixmap = _ozf.tile(x, y);
				if (!pixmap.isNull())
					QPixmapCache::insert(key, pixmap);
			}

			if (pixmap.isNull()) {
				qWarning("%s: error loading tile image", qPrintable(key));
				painter->fillRect(QRectF(QPoint(x, y), _tileSize), Qt::white);
			} else
				painter->drawPixmap(QPoint(x, y), pixmap);
		}
	}
}

void OfflineMap::drawImage(QPainter *painter, const QRectF &rect)
{
	if (!_img || _img->isNull())
		painter->fillRect(rect, Qt::white);
	else {
		QPoint p = rect.topLeft().toPoint();
		QImage crop = _img->copy(QRect(p, rect.size().toSize()));
		painter->drawImage(rect.topLeft(), crop);
	}
}

void OfflineMap::draw(QPainter *painter, const QRectF &rect)
{
	if (_ozf.isOpen())
		drawOZF(painter, rect);
	else if (_tileSize.isValid())
		drawTiled(painter, rect);
	else
		drawImage(painter, rect);
}
