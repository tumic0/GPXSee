#include <QPainter>
#include "config.h"
#include "image.h"


Image::Image(const QString &fileName) : _img(fileName)
{
}

void Image::draw(QPainter *painter, const QRectF &rect, Map::Flags flags)
{
#ifdef ENABLE_HIDPI
	qreal ratio = _img.devicePixelRatioF();
#else // ENABLE_HIDPI
	qreal ratio = 1.0;
#endif // ENABLE_HIDPI
	QRectF sr(rect.topLeft() * ratio, rect.size() * ratio);

	if (flags & Map::OpenGL) {
		QImage img(_img.copy(sr.toRect()));
		painter->drawImage(rect.topLeft(), img);
	} else
		painter->drawImage(rect.topLeft(), _img, sr);
}

void Image::setDevicePixelRatio(qreal ratio)
{
#ifdef ENABLE_HIDPI
	_img.setDevicePixelRatio(ratio);
#endif // ENABLE_HIDPI
}
