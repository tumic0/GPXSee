#include <QPainter>
#include "image.h"

#define TILE_SIZE 256

void Image::draw(QPainter *painter, const QRectF &rect, Map::Flags flags)
{
	qreal ratio = _img.devicePixelRatioF();
	QRectF sr(rect.topLeft() * ratio, rect.size() * ratio);

	/* When OpenGL is used, big images are rendered incredibly slow or not at
	   all using the QPainter::drawImage() function with a source rect set. So
	   we have to tile the image ourself before it can be drawn. */
	if (flags & Map::OpenGL) {
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
