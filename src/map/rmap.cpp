#include <QFileInfo>
#include <QDataStream>
#include <QPixmapCache>
#include <QPainter>
#include <QRegExp>
#include "common/rectc.h"
#include "common/wgs84.h"
#include "common/config.h"
#include "calibrationpoint.h"
#include "utm.h"
#include "pcs.h"
#include "rectd.h"
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
		case 0:
			if (fields.size() < 4)
				return Projection();
			zone = fields.at(2).toInt(&ret);
			if (!ret)
				return Projection();
			if (fields.at(3) == "S")
				zone = -zone;
			pcs = PCS(gcs, 9807, UTM::setup(zone), 9001);
			return Projection(&pcs);
		case 1:
			return Projection(gcs);
		case 2:
			pcs = PCS(gcs, 1024, Projection::Setup(), 9001);
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
	QRegExp re("^P[0-9]+=");

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
  : Map(parent), _mapRatio(1.0), _fileName(fileName), _zoom(0), _valid(false)
{
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly)) {
		_errorString = file.errorString();
		return;
	}

	QDataStream stream(&file);
	stream.setByteOrder(QDataStream::LittleEndian);

	char magic[sizeof(MAGIC) - 1];
	if (stream.readRawData(magic, sizeof(magic)) != sizeof(magic)
	  || memcmp(MAGIC, magic, sizeof(magic))) {
		_errorString = "Not a raster RMap file";
		return;
	}

	quint32 tmp, width, height;
	stream >> tmp >> tmp >> tmp;
	stream >> width >> height;
	QSize imageSize(width, -height);
	stream >> tmp >> tmp;
	stream >> width >> height;
	_tileSize = QSize(width, height);
	quint64 IMPOffset;
	stream >> IMPOffset;
	stream >> tmp;
	qint32 zoomCount;
	stream >> zoomCount;
	CHECK(stream.status() == QDataStream::Ok);

	QVector<quint64> zoomOffsets(zoomCount);
	for (int i = 0; i < zoomCount; i++)
		stream >> zoomOffsets[i];
	CHECK(stream.status() == QDataStream::Ok);

	for (int i = 0; i < zoomOffsets.size(); i++) {
		_zooms.append(Zoom());
		Zoom &zoom = _zooms.last();

		CHECK(file.seek(zoomOffsets.at(i)));

		quint32 width, height;
		stream >> width >> height;
		zoom.size = QSize(width, -height);
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

	CHECK(file.seek(IMPOffset));
	quint32 IMPSize;
	stream >> tmp >> IMPSize;
	CHECK(stream.status() == QDataStream::Ok);

	QByteArray IMP(IMPSize + 1, 0);
	stream.readRawData(IMP.data(), IMP.size());
	_valid = parseIMP(IMP);
}

QString RMap::name() const
{
	QFileInfo fi(_fileName);
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
	_file.setFileName(_fileName);
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
	quint32 tag, len;
	stream >> tag >> len;
	if (stream.status() != QDataStream::Ok)
		return QPixmap();

	QByteArray ba;
	ba.resize(len);
	if (stream.readRawData(ba.data(), ba.size()) != ba.size())
		return QPixmap();

	QImage img(QImage::fromData(ba));
	return QPixmap::fromImage(img);
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
			QString key = _fileName + "/" + QString::number(_zoom) + "_"
			  + QString::number(x) + "_" + QString::number(y);
			if (!QPixmapCache::find(key, &pixmap)) {
				pixmap = tile(x, y);
				if (!pixmap.isNull())
					QPixmapCache::insert(key, pixmap);
			}

			if (pixmap.isNull())
				qWarning("%s: error loading tile image", qPrintable(key));
			else {
#ifdef ENABLE_HIDPI
				pixmap.setDevicePixelRatio(_mapRatio);
#endif // ENABLE_HIDPI
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
