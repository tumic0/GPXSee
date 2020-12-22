#include <QPainter>
#include "image.h"


#define TILE_SIZE 256

void Image::draw(QPainter *painter, const QRectF &rect, Map::Flags flags)
{
	qreal ratio = _img.devicePixelRatioF();
	QRectF sr(rect.topLeft() * ratio, rect.size() * ratio);

	/* When OpenGL is used, big images are rendered incredibly slow or not at
	   all using the QPainter::drawImage() function with a source rect set. So
	   we have to tile the image ourself before it can be drawn.

	   We have to use a list of dynamically allocated pixmaps as QPainter
	   rendering is broken in yet another way with OpenGL and drawPixmap() does
	   access already deleted image instances when reusing a single pixmap. */
	if (flags & Map::OpenGL) {
		QList<QPixmap *> list;

		for (int i = sr.left()/TILE_SIZE; i <= sr.right()/TILE_SIZE; i++) {
			for (int j = sr.top()/TILE_SIZE; j <= sr.bottom()/TILE_SIZE; j++) {
				QPoint tl(i * TILE_SIZE, j * TILE_SIZE);
				QRect tile(tl, QSize(TILE_SIZE, TILE_SIZE));
				QPixmap *pm = new QPixmap(QPixmap::fromImage(_img.copy(tile)));
				list.append(pm);
				pm->setDevicePixelRatio(ratio);
				painter->drawPixmap(tl/ratio, *pm);
			}
		}

		qDeleteAll(list);
	} else
		painter->drawImage(rect.topLeft(), _img, sr);
}

void Image::setDevicePixelRatio(qreal ratio)
{
	_img.setDevicePixelRatio(ratio);
}
