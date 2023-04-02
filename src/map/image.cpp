#include <QPainter>
#include "image.h"

#define TILE_SIZE 256
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#define OPENGL_SIZE_LIMIT 536870912 /* 512MB */
#else
#define OPENGL_SIZE_LIMIT 134217728 /* 128MB */
#endif

void Image::draw(QPainter *painter, const QRectF &rect, Map::Flags flags)
{
	qreal ratio = _img.devicePixelRatioF();
	QRectF sr(rect.topLeft() * ratio, rect.size() * ratio);

	/* When OpenGL is used, big images are rendered incredibly slow or not at
	   all using the QPainter::drawImage() function with a source rect set. So
	   we have to tile the image ourself before it can be drawn. */
	if (flags & Map::OpenGL && _img.sizeInBytes() > OPENGL_SIZE_LIMIT) {
		for (int i = sr.left()/TILE_SIZE; i <= sr.right()/TILE_SIZE; i++) {
			for (int j = sr.top()/TILE_SIZE; j <= sr.bottom()/TILE_SIZE; j++) {
				QPoint tl(i * TILE_SIZE, j * TILE_SIZE);
				QRect tile(tl, QSize(TILE_SIZE, TILE_SIZE));
				QImage img(_img.copy(tile));
				img.setDevicePixelRatio(ratio);
				painter->drawImage(tl/ratio, img);
			}
		}
	} else
		painter->drawImage(rect.topLeft(), _img, sr);
}

void Image::setDevicePixelRatio(qreal ratio)
{
	_img.setDevicePixelRatio(ratio);
}
