#include <QPainter>
#include "config.h"
#include "image.h"


#define TILE_SIZE 256

Image::Image(const QString &fileName) : _img(fileName), _ratio(1.0)
{
}

void Image::draw(QPainter *painter, const QRectF &rect, Map::Flags flags)
{
	QRectF sr(rect.topLeft() * _ratio, rect.size() * _ratio);

	if (flags & Map::OpenGL) {
		QImage img(_img.copy(sr.toRect()));
		painter->drawImage(rect.topLeft(), img);
	} else
		painter->drawImage(rect.topLeft(), _img, sr);
}

void Image::setDevicePixelRatio(qreal ratio)
{
#ifdef ENABLE_HIDPI
	_ratio = ratio;
	_img.setDevicePixelRatio(_ratio);
#endif // ENABLE_HIDPI
}
