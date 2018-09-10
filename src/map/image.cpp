#include <QPainter>
#include <QPixmapCache>
#include "config.h"
#include "image.h"


#define TILE_SIZE 256

Image::Image(const QString &fileName) : _img(fileName), _fileName(fileName)
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
		for (int i = sr.left()/TILE_SIZE; i <= sr.right()/TILE_SIZE; i++) {
			for (int j = sr.top()/TILE_SIZE; j <= sr.bottom()/TILE_SIZE; j++) {
				QString key = _fileName + "-" + QString::number(i) + "_"
				  + QString::number(j);
				QPoint tl(i * TILE_SIZE, j * TILE_SIZE);
				QPixmap pm;

				if (!QPixmapCache::find(key, &pm)) {
					QRect tile(tl, QSize(TILE_SIZE, TILE_SIZE));
					pm = QPixmap::fromImage(_img.copy(tile));
					if (!pm.isNull())
						QPixmapCache::insert(key, pm);
				}
#ifdef ENABLE_HIDPI
				pm.setDevicePixelRatio(ratio);
#endif // ENABLE_HIDPI
				painter->drawPixmap(tl/ratio, pm);
			}
		}
	} else
		painter->drawImage(rect.topLeft(), _img, sr);
}

void Image::setDevicePixelRatio(qreal ratio)
{
#ifdef ENABLE_HIDPI
	_img.setDevicePixelRatio(ratio);
#else // ENABLE_HIDPI
	Q_UNUSED(ratio);
#endif // ENABLE_HIDPI
}
