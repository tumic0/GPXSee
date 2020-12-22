#include <QFileInfo>
#include <QDataStream>
#include <QPixmapCache>
#include <QPainter>
#include <QRegularExpression>
#include <QtEndian>
#include "common/rectc.h"
#include "common/wgs84.h"
#include "calibrationpoint.h"
#include "utm.h"
#include "pcs.h"
#include "rectd.h"
#include "color.h"
#include "rmap.h"


#define MAGIC "CompeGPSRasterImage"
#define CHECK(condition) \
	if (!(condition)) { \
		_errorString = "Invalid/corrupted RMap file"; \
		return; \
	}

static CalibrationPoint parseCalibrationPoint(const QString &str)
{
	QStringList fields(str.split(","));
	if (fields.size() != 5)
		return CalibrationPoint();

	bool ret1, ret2;
	PointD xy(fields.at(0).toDouble(&ret1), fields.at(1).toDouble(&ret2));
	if (!ret1 || !ret2)
		return CalibrationPoint();
	PointD pp(fields.at(3).toDouble(&ret1), fields.at(4).toDouble(&ret2));
	if (!ret1 || !ret2)
		return CalibrationPoint();

	return (fields.at(2) == "A")
	  ? CalibrationPoint(xy, Coordinates(pp.x(), pp.y()))
	  : CalibrationPoint(xy, pp);
}

static Projection parseProjection(const QString &str, const GCS *gcs)
{
	QStringList fields(str.split(","));
	if (fields.isEmpty())
		return Projection();
	bool ret;
	int id = fields.at(0).toDouble(&ret);
	if (!ret)
		return Projection();
	PCS pcs;
	int zone;

	switch (id) {
		case 0: // UTM
			if (fields.size() < 4)
				return Projection();
			zone = fields.at(2).toInt(&ret);
			if (!ret)
				return Projection();
			if (fields.at(3) == "S")
				zone = -zone;
			pcs = PCS(gcs, 9807, UTM::setup(zone), 9001);
			return Projection(&pcs);
		case 1: // LatLon
			return Projection(gcs);
		case 2: // Mercator
			pcs = PCS(gcs, 1024, Projection::Setup(), 9001);
			return Projection(&pcs);
		case 3: // Transversal Mercator
			if (fields.size() < 7)
				return Projection();
			pcs = PCS(gcs, 9807, Projection::Setup(fields.at(3).toDouble(),
			  fields.at(2).toDouble(), fields.at(6).toDouble(),
			  fields.at(5).toDouble(), fields.at(4).toDouble(),
			  NAN, NAN), 9001);
			return Projection(&pcs);
		case 4: // Lambert 2SP
			if (fields.size() < 8)
				return Projection();
			pcs = PCS(gcs, 9802, Projection::Setup(fields.at(4).toDouble(),
			  fields.at(5).toDouble(), NAN,
			  fields.at(6).toDouble(), fields.at(7).toDouble(),
			  fields.at(3).toDouble(), fields.at(2).toDouble()), 9001);
			return Projection(&pcs);
		case 6: // BGN (British National Grid)
			pcs = PCS(gcs, 9807, Projection::Setup(49, -2, 0.999601, 400000,
			  -100000, NAN, NAN), 9001);
			return Projection(&pcs);
		case 12: // France Lambert II etendu
			pcs = PCS(gcs, 9801, Projection::Setup(52, 0, 0.99987742, 600000,
			  2200000, NAN, NAN), 9001);
			return Projection(&pcs);
		case 14: // Swiss Grid
			pcs = PCS(gcs, 9815, Projection::Setup(46.570866, 7.26225, 1.0,
			  600000, 200000, 90.0, 90.0), 9001);
			return Projection(&pcs);
		case 108: // Dutch RD grid
			pcs = PCS(gcs, 9809, Projection::Setup(52.15616055555555,
			  5.38763888888889, 0.9999079, 155000, 463000, NAN, NAN), 9001);
			return Projection(&pcs);
		case 184: // Swedish Grid
			pcs = PCS(gcs, 9807, Projection::Setup(0, 15.808278, 1, 1500000, 0,
			  NAN, NAN), 9001);
			return Projection(&pcs);
		default:
			return Projection();
	}
}

bool RMap::parseIMP(const QByteArray &data)
{
	QStringList lines = QString(data).split("\r\n");
	QVector<CalibrationPoint> calibrationPoints;
	const GCS *gcs = 0;
	QString projection, datum;
	QRegularExpression re("^P[0-9]+=");

	for (int i = 0; i < lines.count(); i++) {
		const QString &line = lines.at(i);

		if (line.startsWith("Projection="))
			projection = line.split("=").at(1);
		else if (line.startsWith("Datum="))
			datum = line.split("=").at(1);
		else if (line.contains(re)) {
			QString point(line.split("=").at(1));
			CalibrationPoint cp(parseCalibrationPoint(point));
			if (cp.isValid())
				calibrationPoints.append(cp);
			else {
				_errorString = point + ": invalid calibration point";
				return false;
			}
		}
	}

	if (!(gcs = GCS::gcs(datum))) {
		_errorString = datum + ": unknown/invalid datum";
		return false;
	}
	_projection = parseProjection(projection, gcs);
	if (!_projection.isValid()) {
		_errorString = projection + ": unknown/invalid projection";
		return false;
	}

	QList<ReferencePoint> rp;
	for (int i = 0; i < calibrationPoints.size(); i++)
		rp.append(calibrationPoints.at(i).rp(_projection));

	_transform = Transform(rp);
	if (!_transform.isValid()) {
		_errorString = _transform.errorString();
		return false;
	}

	return true;
}

RMap::RMap(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _file(fileName), _mapRatio(1.0), _zoom(0),
  _valid(false)
{
	if (!_file.open(QIODevice::ReadOnly)) {
		_errorString = _file.errorString();
		return;
	}

	QDataStream stream(&_file);
	stream.setByteOrder(QDataStream::LittleEndian);

	char magic[sizeof(MAGIC) - 1];
	if (stream.readRawData(magic, sizeof(magic)) != sizeof(magic)
	  || memcmp(MAGIC, magic, sizeof(magic))) {
		_errorString = "Not a raster RMap file";
		return;
	}

	quint32 unknown, type, subtype, obfuscated, width, height, bpp, tileWidth,
	  tileHeight, paletteSize;
	quint64 IMPOffset;

	stream >> type;
	if (type > 5)
		stream >> subtype >> obfuscated;
	else
		obfuscated = 0;
	stream >> width >> height >> bpp >> unknown >> tileWidth >> tileHeight
	  >> IMPOffset >> paletteSize;
	CHECK(stream.status() == QDataStream::Ok);

	if (!(type == 5 || (type >= 8 && type <= 10))) {
		_errorString = QString::number(type) + ": unsupported map type";
		return;
	}
	if (obfuscated) {
		_errorString = "Obfuscated maps not supported";
		return;
	}

	QSize imageSize(width, -(int)height);
	_tileSize = QSize(tileWidth, tileHeight);

	if (paletteSize) {
		quint32 bgr;
		CHECK(paletteSize <= 256);

		_palette.resize(256);
		for (int i = 0; i < (int)paletteSize; i++) {
			stream >> bgr;
			_palette[i] = Color::bgr2rgb(bgr);
		}
	}

	qint32 zoomCount;
	stream >> zoomCount;
	CHECK(stream.status() == QDataStream::Ok && zoomCount);

	QVector<quint64> zoomOffsets(zoomCount);
	for (int i = 0; i < zoomCount; i++)
		stream >> zoomOffsets[i];
	CHECK(stream.status() == QDataStream::Ok);

	for (int i = 0; i < zoomOffsets.size(); i++) {
		_zooms.append(Zoom());
		Zoom &zoom = _zooms.last();

		CHECK(_file.seek(zoomOffsets.at(i)));

		quint32 width, height;
		stream >> width >> height;
		zoom.size = QSize(width, -(int)height);
		stream >> width >> height;
		zoom.dim = QSize(width, height);
		zoom.scale = QPointF((qreal)zoom.size.width() / (qreal)imageSize.width(),
		  (qreal)zoom.size.height() / (qreal)imageSize.height());
		CHECK(stream.status() == QDataStream::Ok);

		zoom.tiles.resize(zoom.dim.width() * zoom.dim.height());
		for (int j = 0; j < zoom.tiles.size(); j++)
			stream >> zoom.tiles[j];
		CHECK(stream.status() == QDataStream::Ok);
	}

	CHECK(_file.seek(IMPOffset));
	quint32 IMPSize;
	stream >> unknown >> IMPSize;
	CHECK(stream.status() == QDataStream::Ok);

	QByteArray IMP(IMPSize + 1, 0);
	stream.readRawData(IMP.data(), IMP.size());
	_valid = parseIMP(IMP);

	_file.close();
}

QString RMap::name() const
{
	QFileInfo fi(path());
	return fi.baseName();
}

QRectF RMap::bounds()
{
	return QRectF(QPointF(0, 0), _zooms.at(_zoom).size / _mapRatio);
}

int RMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (!rect.isValid())
		_zoom = 0;
	else {
		RectD prect(rect, _projection);
		QRectF sbr(_transform.proj2img(prect.topLeft()),
		  _transform.proj2img(prect.bottomRight()));

		for (int i = 0; i < _zooms.size(); i++) {
			_zoom = i;
			const Zoom &z = _zooms.at(i);
			if (sbr.size().width() * z.scale.x() <= size.width()
			  && sbr.size().height() * z.scale.y() <= size.height())
				break;
		}
	}

	return _zoom;
}

int RMap::zoomIn()
{
	_zoom = qMax(_zoom - 1, 0);
	return _zoom;
}

int RMap::zoomOut()
{
	_zoom = qMin(_zoom + 1, _zooms.size() - 1);
	return _zoom;
}

QPointF RMap::ll2xy(const Coordinates &c)
{
	const QPointF &scale = _zooms.at(_zoom).scale;
	QPointF p(_transform.proj2img(_projection.ll2xy(c)));
	return QPointF(p.x() * scale.x(), p.y() * scale.y()) / _mapRatio;
}

Coordinates RMap::xy2ll(const QPointF &p)
{
	const QPointF &scale = _zooms.at(_zoom).scale;
	return  _projection.xy2ll(_transform.img2proj(QPointF(p.x() / scale.x(),
	  p.y() / scale.y()) * _mapRatio));
}

void RMap::load()
{
	_file.open(QIODevice::ReadOnly);
}

void RMap::unload()
{
	_file.close();
}

QPixmap RMap::tile(int x, int y)
{
	const Zoom &zoom = _zooms.at(_zoom);

	qint32 index = y / _tileSize.height() * zoom.dim.width()
	  + x / _tileSize.width();
	if (index > zoom.tiles.size())
		return QPixmap();

	quint64 offset = zoom.tiles.at(index);
	if (!_file.seek(offset))
		return QPixmap();
	QDataStream stream(&_file);
	stream.setByteOrder(QDataStream::LittleEndian);
	quint32 tag;
	stream >> tag;
	if (stream.status() != QDataStream::Ok)
		return QPixmap();

	if (tag == 2) {
		if (_palette.isEmpty())
			return QPixmap();
		quint32 width, height, size;
		stream >> width >> height >> size;
		QSize tileSize(width, -(int)height);

		quint32 bes = qToBigEndian(tileSize.width() * tileSize.height());
		QByteArray ba;
		ba.resize(sizeof(bes) + size);
		memcpy(ba.data(), &bes, sizeof(bes));

		if (stream.readRawData(ba.data() + sizeof(bes), size) != (int)size)
			return QPixmap();
		QByteArray uba = qUncompress(ba);
		if (uba.size() < tileSize.width() * tileSize.height())
			return QPixmap();
		QImage img((const uchar*)uba.constData(), tileSize.width(),
		  tileSize.height(), QImage::Format_Indexed8);
		img.setColorTable(_palette);

		return QPixmap::fromImage(img);
	} else if (tag == 7) {
		quint32 len;
		stream >> len;

		QByteArray ba;
		ba.resize(len);
		if (stream.readRawData(ba.data(), ba.size()) != ba.size())
			return QPixmap();

		QImage img(QImage::fromData(ba));
		return QPixmap::fromImage(img);
	} else
		return QPixmap();
}

void RMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	Q_UNUSED(flags);

	QSizeF ts(_tileSize.width() / _mapRatio, _tileSize.height() / _mapRatio);
	QPointF tl(floor(rect.left() / ts.width()) * ts.width(),
	  floor(rect.top() / ts.height()) * ts.height());

	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	for (int i = 0; i < ceil(s.width() / ts.width()); i++) {
		for (int j = 0; j < ceil(s.height() / ts.height()); j++) {
			int x = round(tl.x() * _mapRatio + i * _tileSize.width());
			int y = round(tl.y() * _mapRatio + j * _tileSize.height());

			QPixmap pixmap;
			QString key = path() + "/" + QString::number(_zoom) + "_"
			  + QString::number(x) + "_" + QString::number(y);
			if (!QPixmapCache::find(key, &pixmap)) {
				pixmap = tile(x, y);
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

void RMap::setDevicePixelRatio(qreal deviceRatio, qreal mapRatio)
{
	Q_UNUSED(deviceRatio);
	_mapRatio = mapRatio;
}
