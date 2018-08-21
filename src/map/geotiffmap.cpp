#include <QFileInfo>
#include <QPainter>
#include <QImageReader>
#include "config.h"
#include "geotiff.h"
#include "geotiffmap.h"


GeoTIFFMap::GeoTIFFMap(const QString &fileName, QObject *parent)
  : Map(parent), _img(0), _ratio(1.0), _valid(false)
{
	GeoTIFF gt(fileName);
	if (!gt.isValid()) {
		_errorString = gt.errorString();
		return;
	} else {
		_path = fileName;
		_projection = gt.projection();
		_transform = gt.transform();
	}

	QImageReader img(_path);
	_size = img.size();
	if (!_size.isValid()) {
		_errorString = QString("%1: Invalid image file").arg(_path);
		return;
	}

	_valid = true;
}

GeoTIFFMap::~GeoTIFFMap()
{
	delete _img;
}

QString GeoTIFFMap::name() const
{
	QFileInfo fi(_path);
	return fi.fileName();
}

void GeoTIFFMap::load()
{
	if (!_img) {
		_img = new QImage(_path);
		if (!_img || _img->isNull()) {
			qWarning("%s: error loading map image", qPrintable(_path));
			return;
		}
#ifdef ENABLE_HIDPI
		_img->setDevicePixelRatio(_ratio);
#endif // ENABLE_HIDPI
	}
}

void GeoTIFFMap::unload()
{
	delete _img;
	_img = 0;
}

QPointF GeoTIFFMap::ll2xy(const Coordinates &c)
{
	return QPointF(_transform.proj2img(_projection.ll2xy(c))) / _ratio;
}

Coordinates GeoTIFFMap::xy2ll(const QPointF &p)
{
	return _projection.xy2ll(_transform.img2proj(p * _ratio));
}

QRectF GeoTIFFMap::bounds()
{
	return QRectF(QPointF(0, 0), _size / _ratio);
}

void GeoTIFFMap::draw(QPainter *painter, const QRectF &rect, bool block)
{
	Q_UNUSED(block)

	if (_img && !_img->isNull()) {
		/* Drawing directly a sub-rectangle without an image copy does not work
		   for big images under OpenGL. The image is most probably loaded as
		   whole which exceeds the texture size limit. */
		QRectF sr(rect.topLeft() * _ratio, rect.size() * _ratio);
		QImage img(_img->copy(sr.toRect()));
		painter->drawImage(rect.topLeft(), img);
	}
}
