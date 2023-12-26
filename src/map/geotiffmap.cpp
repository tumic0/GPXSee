#include <QPainter>
#include <QImageReader>
#include "geotiff.h"
#include "image.h"
#include "geotiffmap.h"


GeoTIFFMap::GeoTIFFMap(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _img(0), _ratio(1.0), _valid(false)
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

void GeoTIFFMap::load(const Projection &in, const Projection &out,
  qreal deviceRatio, bool hidpi)
{
	Q_UNUSED(in);
	Q_UNUSED(out);

	_ratio = hidpi ? deviceRatio : 1.0;

	_img = new Image(path());
	if (_img)
		_img->setDevicePixelRatio(_ratio);
}

void GeoTIFFMap::unload()
{
	delete _img;
	_img = 0;
}

Map *GeoTIFFMap::create(const QString &path, const Projection &proj, bool *isDir)
{
	Q_UNUSED(proj);

	if (isDir)
		*isDir = false;

	return new GeoTIFFMap(path);
}
