#include <QPainter>
#include <QPixmapCache>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QXmlStreamReader>
#include "pcs.h"
#include "utm.h"
#include "oruxmap.h"


#define META_TYPE(type) static_cast<QMetaType::Type>(type)

static bool intAttr(QXmlStreamReader &reader, const QXmlStreamAttributes &attr,
  const QString &name, int &val)
{
	bool ok;
	if (!attr.hasAttribute(name)) {
		reader.raiseError(QString("Missing %1 attribute").arg(name));
		return false;
	}
	val = attr.value(name).toInt(&ok);
	if (!ok) {
		reader.raiseError(QString("Invalid %1 attribute").arg(name));
		return false;
	}

	return true;
}

static bool dblAttr(QXmlStreamReader &reader, const QXmlStreamAttributes &attr,
  const QString &name, double &val)
{
	bool ok;
	if (!attr.hasAttribute(name)) {
		reader.raiseError(QString("Missing %1 attribute").arg(name));
		return false;
	}
	val = attr.value(name).toDouble(&ok);
	if (!ok) {
		reader.raiseError(QString("Invalid %1 attribute").arg(name));
		return false;
	}

	return true;
}

static bool strAttr(QXmlStreamReader &reader, const QXmlStreamAttributes &attr,
  const QString &name, QString &val)
{
	if (!attr.hasAttribute(name)) {
		reader.raiseError(QString("Missing %1 attribute").arg(name));
		return false;
	}

	val = attr.value(name).toString();

	return true;
}

static PointD corner2point(const QString &name, const QSize &size)
{
	if (name == "TL")
		return PointD(0, 0);
	else if (name == "BR")
		return PointD(size.width(), size.height());
	else if (name == "TR")
		return PointD(size.width(), 0);
	else if (name == "BL")
		return PointD(0, size.height());
	else
		return PointD();
}

static Projection::Setup lcc2setup(const QStringList &list)
{
	double params[6];
	bool ok;

	if (list.size() < 7)
		return Projection::Setup();
	for (int i = 1; i < 7; i++) {
		params[i - 1] = list.at(i).toDouble(&ok);
		if (!ok)
			return Projection::Setup();
	}

	return Projection::Setup(params[0], params[1], NAN, params[4],
	  params[5], params[2], params[3]);
}

static Projection::Setup laea2setup(const QStringList &list)
{
	double params[2];
	bool ok;

	if (list.size() < 3)
		return Projection::Setup();
	for (int i = 1; i < 3; i++) {
		params[i - 1] = list.at(i).toDouble(&ok);
		if (!ok)
			return Projection::Setup();
	}

	return Projection::Setup(params[1], params[0], NAN, 0, 0, NAN, NAN);
}

static Projection::Setup polyconic2setup(const QStringList &list)
{
	double params[3];
	bool ok;

	if (list.size() < 4)
		return Projection::Setup();
	for (int i = 1; i < 4; i++) {
		params[i - 1] = list.at(i).toDouble(&ok);
		if (!ok)
			return Projection::Setup();
	}

	return Projection::Setup(NAN, params[0], NAN, params[1], params[2], NAN,
	  NAN);
}

static Projection::Setup tm2setup(const QStringList &list)
{
	double params[5];
	bool ok;

	if (list.size() < 6)
		return Projection::Setup();
	for (int i = 1; i < 6; i++) {
		params[i - 1] = list.at(i).toDouble(&ok);
		if (!ok)
			return Projection::Setup();
	}

	return Projection::Setup(params[1], params[0], params[2], params[3],
	  params[4], NAN, NAN);
}

static Projection::Setup utm2setup(const QStringList &list)
{
	bool ok;
	if (list.size() < 2)
		return Projection::Setup();
	int zone = list.at(1).toInt(&ok);
	return ok ? UTM::setup(zone) : Projection::Setup();
}

static Projection::Setup mercator2setup(const QStringList &list)
{
	double lon;
	bool ok;

	if (list.size() < 2)
		return Projection::Setup(0, 0, NAN, 0, 0, NAN, NAN);
	lon = list.at(1).toDouble(&ok);

	return ok ? Projection::Setup(0, lon, NAN, 0, 0, NAN, NAN)
	  : Projection::Setup();
}

static GCS createGCS(const QString &datum)
{
	QStringList dl(datum.split(':'));
	return (GCS::gcs(dl.first()));
}

static Projection createProjection(const GCS &gcs, const QString &name)
{
	PCS pcs;
	QStringList pl(name.split(','));

	if (pl.first() == "Latitude/Longitude")
		return Projection(gcs);
	else if (pl.first() == "UTM")
		pcs = PCS(gcs, 9807, utm2setup(pl), 9001);
	else if (pl.first() == "Mercator")
		pcs = PCS(gcs, 1024, Projection::Setup(), 9001);
	else if (pl.first() == "Mercator Ellipsoidal")
		pcs = PCS(gcs, 9804, mercator2setup(pl), 9001);
	else if (pl.first() == "Transverse Mercator")
		pcs = PCS(gcs, 9807, tm2setup(pl), 9001);
	else if (pl.first() == "Lambert Conformal Conic")
		pcs = PCS(gcs, 9802, lcc2setup(pl), 9001);
	else if (pl.first() == "(A)Lambert Azimuthual Equal Area")
		pcs = PCS(gcs, 9820, laea2setup(pl), 9001);
	else if (pl.first() == "Polyconic (American)")
		pcs = PCS(gcs, 9818, polyconic2setup(pl), 9001);
	else if (pl.first() == "(IG) Irish Grid")
		pcs = PCS(gcs, 9807, Projection::Setup(53.5, -8, 1.000035, 200000,
		  250000, NAN, NAN), 9001);
	else if (pl.first() == "(SUI) Swiss Grid")
		pcs = PCS(gcs, 9815, Projection::Setup(46.570866, 7.26225, 1.0, 600000,
		  200000, 90.0, 90.0), 9001);
	else if (pl.first() == "Rijksdriehoeksmeting")
		pcs = PCS(gcs, 9809, Projection::Setup(52.1561605555556,
		  5.38763888888889, 0.9999079, 155000, 463000, NAN, NAN), 9001);
	else
		return Projection();

	return Projection(pcs);
}

static Transform computeTransformation(const Projection &proj,
  const QList<CalibrationPoint> &points)
{
	QList<ReferencePoint> rp;

	for (int i = 0; i < points.size(); i++)
		rp.append(points.at(i).rp(proj));

	return Transform(rp);
}

void OruxMap::calibrationPoints(QXmlStreamReader &reader, const QSize &size,
  QList<CalibrationPoint> &points)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("CalibrationPoint")) {
			double lon, lat;
			QString corner;

			QXmlStreamAttributes attr = reader.attributes();
			if (!dblAttr(reader, attr, "lat", lat))
				return;
			if (!dblAttr(reader, attr, "lon", lon))
				return;
			if (!strAttr(reader, attr, "corner", corner))
				return;

			CalibrationPoint p(corner2point(corner, size), Coordinates(lon, lat));
			if (!p.isValid()) {
				reader.raiseError(QString("invalid calibration point"));
				return;
			}
			points.append(p);

			reader.readElementText();
		} else
			reader.skipCurrentElement();
	}
}

void OruxMap::mapCalibration(QXmlStreamReader &reader, const QString &dir,
  int level)
{
	int zoom;
	QSize tileSize, size, calibrationSize;
	QString fileName;
	Projection proj;
	Transform t;

	QXmlStreamAttributes attr = reader.attributes();
	if (!intAttr(reader, attr, "layerLevel", zoom))
		return;

	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("OruxTracker"))
			oruxTracker(reader, dir, level + 1);
		else if (reader.name() == QLatin1String("MapName")) {
			QString name(reader.readElementText());
			if (!level && dir.isEmpty())
				_name = name;
		} else if (reader.name() == QLatin1String("MapChunks")) {
			int xMax, yMax, width, height;
			QString datum, projection;

			QXmlStreamAttributes attr = reader.attributes();
			if (!intAttr(reader, attr, "xMax", xMax))
				return;
			if (!intAttr(reader, attr, "yMax", yMax))
				return;
			if (!intAttr(reader, attr, "img_width", width))
				return;
			if (!intAttr(reader, attr, "img_height", height))
				return;
			if (!strAttr(reader, attr, "datum", datum))
				return;
			if (!strAttr(reader, attr, "projection", projection))
				return;
			if (!strAttr(reader, attr, "file_name", fileName))
				return;

			tileSize = QSize(width, height);
			size = QSize(xMax * width, yMax * height);
			calibrationSize = size;

			GCS gcs(createGCS(datum));
			if (!gcs.isValid()) {
				reader.raiseError(QString("%1: invalid/unknown datum")
				  .arg(datum));
				return;
			}
			proj = createProjection(gcs, projection);
			if (!proj.isValid()) {
				reader.raiseError(QString("%1: invalid/unknown projection")
				  .arg(projection));
				return;
			}

			reader.readElementText();

		} else if (reader.name() == QLatin1String("MapDimensions")) {
			int height, width;

			QXmlStreamAttributes attr = reader.attributes();
			if (!intAttr(reader, attr, "height", height))
				return;
			if (!intAttr(reader, attr, "width", width))
				return;

			calibrationSize = QSize(width, height);

			reader.readElementText();
		} else if (reader.name() == QLatin1String("CalibrationPoints")) {
			QList<CalibrationPoint> points;

			calibrationPoints(reader, calibrationSize, points);

			t = computeTransformation(proj, points);
			if (!t.isValid()) {
				reader.raiseError(t.errorString());
				return;
			}
		} else
			reader.skipCurrentElement();
	}

	if (tileSize.isValid()) {
		if (!t.isValid()) {
			reader.raiseError("Invalid map calibration");
			return;
		}

		QDir mapDir(QFileInfo(path()).absoluteDir());
		QDir subDir = dir.isEmpty()
		  ? mapDir : QDir(mapDir.absoluteFilePath(dir));
		QString set(subDir.absoluteFilePath("set"));

		_zooms.append(Zoom(zoom, tileSize, size, proj, t, fileName, set));
	}
}

void OruxMap::oruxTracker(QXmlStreamReader &reader, const QString &dir,
  int level)
{
	if (level > 1 || (level && !dir.isEmpty())) {
		reader.raiseError("invalid map nesting");
		return;
	}

	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("MapCalibration"))
			mapCalibration(reader, dir, level);
		else
			reader.skipCurrentElement();
	}
}

bool OruxMap::readXML(const QString &path, const QString &dir)
{
	QFile file(path);

	if (!file.open(QFile::ReadOnly | QFile::Text))
		return false;

	QXmlStreamReader reader(&file);
	if (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("OruxTracker"))
			oruxTracker(reader, dir, 0);
		else
			reader.raiseError("Not a Orux map calibration file");
	}
	if (reader.error()) {
		_errorString = QString("%1: %2").arg(reader.lineNumber())
		  .arg(reader.errorString());
		return false;
	}

	return true;
}

OruxMap::OruxMap(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _zoom(0), _mapRatio(1.0), _valid(false)
{
	QFileInfo fi(fileName);
	QDir dir(fi.absoluteDir());

	if (!readXML(fileName))
		return;
	if (_zooms.isEmpty()) {
		QStringList list(dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot));
		for (int i = 0; i < list.size(); i++) {
			QDir subDir(dir.absoluteFilePath(list.at(i)));
			if (!readXML(subDir.absoluteFilePath(list.at(i) + ".otrk2.xml"),
			  list.at(i))) {
				_errorString = list.at(i) + ": " + _errorString;
				return;
			}
		}
		if (_zooms.isEmpty()) {
			_errorString = "No usable zoom level found";
			return;
		}
	}
	std::sort(_zooms.begin(), _zooms.end());


	if (dir.exists("OruxMapsImages.db")) {
		QString dbFile(dir.absoluteFilePath("OruxMapsImages.db"));
		_db = QSqlDatabase::addDatabase("QSQLITE", dbFile);
		_db.setDatabaseName(dbFile);
		_db.setConnectOptions("QSQLITE_OPEN_READONLY");
		if (!_db.open()) {
			_errorString = "Error opening database file";
			return;
		}

		QSqlRecord r = _db.record("tiles");
		if (r.isEmpty()
		  || r.field(0).name() != "x"
		  || META_TYPE(r.field(0).type()) != QMetaType::Int
		  || r.field(1).name() != "y"
		  || META_TYPE(r.field(1).type()) != QMetaType::Int
		  || r.field(2).name() != "z"
		  || META_TYPE(r.field(2).type()) != QMetaType::Int
		  || r.field(3).name() != "image"
		  || META_TYPE(r.field(3).type()) != QMetaType::QByteArray) {
			_errorString = "Invalid table format";
			return;
		}

		_db.close();
	} else {
		for (int i = 0; i < _zooms.size(); i++) {
			if (!_zooms.at(i).set.exists()) {
				_errorString = "missing set directory (level "
				  + QString::number(_zooms.at(i).zoom) + ")";
				return;
			}
		}
	}

	_valid = true;
}

int OruxMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (!rect.isValid())
		_zoom = _zooms.size() - 1;
	else {
		for (int i = 1; i < _zooms.size(); i++) {
			_zoom = i;
			QRect sbr(QPoint(ll2xy(rect.topLeft()).toPoint()),
			  QPoint(ll2xy(rect.bottomRight()).toPoint()));
			if (sbr.size().width() >= size.width() || sbr.size().height()
			  >= size.height()) {
				_zoom--;
				break;
			}
		}
	}

	return _zoom;
}

int OruxMap::zoomIn()
{
	_zoom = qMin(_zoom + 1, _zooms.size() - 1);
	return _zoom;
}

int OruxMap::zoomOut()
{
	_zoom = qMax(_zoom - 1, 0);
	return _zoom;
}

void OruxMap::load()
{
	if (_db.isValid())
		_db.open();
}

void OruxMap::unload()
{
	if (_db.isValid())
		_db.close();
}

void OruxMap::setDevicePixelRatio(qreal deviceRatio, qreal mapRatio)
{
	Q_UNUSED(deviceRatio);
	_mapRatio = mapRatio;
}

QPixmap OruxMap::tile(const Zoom &z, int x, int y) const
{
	if (_db.isValid()) {
		QSqlQuery query(_db);
		query.prepare("SELECT image FROM tiles WHERE z=:z AND x=:x AND y=:y");
		query.bindValue(":z", z.zoom);
		query.bindValue(":x", x);
		query.bindValue(":y", y);
		query.exec();

		if (!query.first()) {
			qWarning("%s: SQL %d-%d-%d: not found", qPrintable(name()), z.zoom,
			  x, y);
			return QPixmap();
		} else {
			QImage img(QImage::fromData(query.value(0).toByteArray()));
			return QPixmap::fromImage(img);
		}
	} else {
		QString fileName(z.fileName + "_" + QString::number(x) + "_"
		  + QString::number(y) + ".omc2");
		QString path(z.set.absoluteFilePath(fileName));
		if (!QFileInfo::exists(path)) {
			qWarning("%s: %s: not found", qPrintable(name()),
			  qPrintable(fileName));
			return QPixmap();
		} else {
			QImage img(path);
			return QPixmap::fromImage(img);
		}
	}
}

void OruxMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	Q_UNUSED(flags);
	const Zoom &z = _zooms.at(_zoom);
	QSizeF ts(z.tileSize.width() / _mapRatio, z.tileSize.height() / _mapRatio);
	QPointF tl(floor(rect.left() / ts.width()) * ts.width(),
	  floor(rect.top() / ts.height()) * ts.height());

	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	for (int i = 0; i < ceil(s.width() / ts.width()); i++) {
		for (int j = 0; j < ceil(s.height() / ts.height()); j++) {
			int x = round(tl.x() * _mapRatio + i * z.tileSize.width());
			int y = round(tl.y() * _mapRatio + j * z.tileSize.height());

			QPixmap pixmap;
			QString key = path() + "/" + QString::number(z.zoom)
			  + "_" + QString::number(x/z.tileSize.width())
			  + "_" + QString::number(y/z.tileSize.height());
			if (!QPixmapCache::find(key, &pixmap)) {
				pixmap = tile(z, x/z.tileSize.width(), y/z.tileSize.height());
				if (!pixmap.isNull())
					QPixmapCache::insert(key, pixmap);
			}

			if (!pixmap.isNull()) {
				pixmap.setDevicePixelRatio(_mapRatio);
				QPointF tp(tl.x() + i * ts.width(), tl.y() + j * ts.height());
				painter->drawPixmap(tp, pixmap);
			}
		}
	}
}

QRectF OruxMap::bounds()
{
	const Zoom &z = _zooms.at(_zoom);
	return QRectF(QPointF(0, 0), z.size / _mapRatio);
}

QPointF OruxMap::ll2xy(const Coordinates &c)
{
	const Zoom &z = _zooms.at(_zoom);
	QPointF p(z.transform.proj2img(z.projection.ll2xy(c)));
	return (p / _mapRatio);
}

Coordinates OruxMap::xy2ll(const QPointF &p)
{
	const Zoom &z = _zooms.at(_zoom);
	return z.projection.xy2ll(z.transform.img2proj(p * _mapRatio));
}

Map *OruxMap::create(const QString &path, const Projection &, bool *isDir)
{
	if (isDir)
		*isDir = true;

	return new OruxMap(path);
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const OruxMap::Zoom &zoom)
{
	dbg.nospace() << "Zoom(" << zoom.zoom << ", " << zoom.tileSize << ", "
	  << zoom.size << ")";

	return dbg.space();
}
#endif // QT_NO_DEBUG
