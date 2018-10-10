#include <QFileInfo>
#include <QPainter>
#include <QImageReader>
#include "config.h"
#include "geotiff.h"
#include "image.h"
#include "geotiffmap.h"


GeoTIFFMap::GeoTIFFMap(const QString &fileName, QObject *parent)
  : Map(parent), _fileName(fileName), _img(0), _ratio(1.0), _valid(false)
{
	QImageReader ir(fileName);
	if (!ir.canRead()) {
		_errorString = "Unsupported/invalid image file";
		return;
	}
	_size = ir.size();

	GeoTIFF gt(fileName);
	if (!gt.isValid()) {
		_errorString = gt.errorString();
		return;
	} else {
		_projection = gt.projection();
		_transform = gt.transform();
	}

	_valid = true;
}

GeoTIFFMap::~GeoTIFFMap()
{
	delete _img;
}

QString GeoTIFFMap::name() const
{
	QFileInfo fi(_fileName);
	return fi.fileName();
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

void GeoTIFFMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	if (_img)
		_img->draw(painter, rect, flags);
}

void GeoTIFFMap::setDevicePixelRatio(qreal ratio)
{
	_ratio = ratio;
	if (_img)
		_img->setDevicePixelRatio(_ratio);
}

void GeoTIFFMap::load()
{
	if (!_img)
		_img = new Image(_fileName);
}

void GeoTIFFMap::unload()
{
	delete _img;
	_img = 0;
}
